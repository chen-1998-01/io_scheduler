#include"../manager/logger.h"
#include"schedule.h"

	static logger::_logger g_logger = root_logger();

	static thread_local schedule* g_cur_schedule = nullptr;
	//表示当前运行的调度器
	static thread_local coroutine* g_cur_coroutine = nullptr;
	//表示调度器上的当前协程

	schedule::schedule(size_t thread_num,const std::string& name,bool use_mainfiber):
	m_root_coroutine(nullptr),m_running(false),m_autostop(false) {
		m_mutex=new Mutex();
		assert(thread_num);
		m_name = name;
		if (use_mainfiber) {
			assert(g_cur_schedule == nullptr);
			g_cur_schedule = this;
			Thread::set_name(m_name);//线程名称采取和调度器相同的名称
			m_root_threadid = get_thread_id();
			m_threads_id.push_back(m_root_threadid);
			coroutine::get_this_coroutine();//加载初始化协程
	    m_root_coroutine.reset(new coroutine(std::bind(&schedule::run, this), (128*1024), true));//设置调度器主协程
			g_cur_coroutine=m_root_coroutine.get();
			thread_num--;
		}//使用root协程
		else {
			m_root_coroutine = nullptr;
			m_root_threadid = -1;
		}//不使用root协程,此时主协程置为空
		m_threadsnum = thread_num;
		logger_inform_eventstream(g_logger)<<"A scheduler been created ";
		record(g_logger,logger_level::inform);
		start();
	}

	schedule::~schedule() {
		if (get_this_schedule() == this) {
			g_cur_schedule = nullptr;			
		}
		logger_inform_eventstream(g_logger)<<"the scheduler been deleted ";
		record(g_logger,logger_level::inform);	
	}//当调度器已经停止运行时才能析构该调度器，如果该调度器恰好为主调度器，则将当期运行调度器置为空

	schedule* schedule::get_this_schedule() {
		return g_cur_schedule;
	}

	void schedule::set_this_schedule(schedule* _schedule) {
		g_cur_schedule = _schedule;
	}


	coroutine* schedule::get_currentcoroutine() {
		return g_cur_coroutine;
	}


	void schedule::start() {
		{
			locallock<Mutex>lock(*m_mutex);//保证线程安全
			if (m_running)
				return;
			m_threads.resize(m_threadsnum);
			for (int i = 0; i < m_threadsnum; i++) {
				m_threads[i].reset(new Thread(std::bind(&schedule::run, this), m_name + std::to_string(i)));
				m_threads_id.push_back(m_threads[i]->get_this_threadid());
			}
		}
	}//建立线程池,开始执行任务

	void schedule::run() {
		logger_inform_eventstream(g_logger)<<"the scheduler is running ";
		record(g_logger,logger_level::inform);
		set_this_schedule(this);//设置当前的调度器
		if (m_root_threadid != get_thread_id()) {
			g_cur_coroutine=coroutine::get_this_coroutine().get();
		}//更新当前协程
		coroutine_s temp;//暂存当前任务
		coroutine::coroutine_ptr idle_coroutine(new coroutine(std::bind(&schedule::idle,this),128*1024,false));
		coroutine::coroutine_ptr temp_coroutine = nullptr;//绑定回调函数并且执行
		while (1) {
			temp.reset();
			bool need_notify = false;
			{
				locallock<Mutex>lock(*m_mutex);
				if(m_coroutines.empty()){
				  break;}
				std::list<schedule::coroutine_s>::iterator i = m_coroutines.begin();
				while (i != m_coroutines.end()){
					if (i->thread_id != -1 && i->thread_id != get_thread_id()) {
						need_notify = true;
						i++;
						continue;
					}
					if (i->s_coroutine && i->s_coroutine->get_state() == coroutine::state::running) {
						i++;
						continue;
					}
					assert(i->s_coroutine || i->s_function);
					temp = *i;
					m_coroutines.erase(i);
					break;
				}
		 }//交给线程池来争夺任务，一旦某个线程争夺到任务，就将资源提供保护，然后遍历任务队列，找到当前线程需要执行的任务
				
				if (need_notify)
					notify();

				if (temp.s_coroutine && temp.s_coroutine->get_state() != coroutine::state::terminate) {
					m_activethreadnum++;
					temp.s_coroutine->swap_enter(); //如果为协程，将其切换为当前运行的协程
					//接着判断运行结束后的状态
					m_activethreadnum--;
					if (temp.s_coroutine->get_state() == coroutine::state::ready) {
						dispatch(temp.s_coroutine);
						temp.reset();
					}//运行结束后当是ready状态时，重新放入到任务队列中
					else if (temp.s_coroutine->get_state() != coroutine::state::terminate && temp.s_coroutine->get_state() != coroutine::state::error) {
						temp.s_coroutine->set_state(coroutine::state::pause);
						//当时间片用完时，将其状态挂起
						temp.reset();
					}
					else {
						temp.reset();
					}
				}

        //如果为回调函数，则将其绑定一个协程,然后加入到当前的线程任务中
				else if (temp.s_function) {
					std::function<void()>temp_function;
					temp_function.swap(temp.s_function);
					if (temp_coroutine)
						temp_coroutine->swap(temp_function);
					else if (temp_coroutine == NULL) {
						temp_coroutine.reset(new coroutine(temp_function, 128 * 1024, true));
					}
					temp.reset();
					m_activethreadnum++;
					temp_coroutine->swap_enter();
					m_activethreadnum--;
					if (temp_coroutine->get_state() == (coroutine::state::error | coroutine::state::terminate)) {
						temp_coroutine->swap(nullptr);
					}
					else if(temp_coroutine->get_state()==coroutine::state::ready){
						dispatch(temp_coroutine);
				    }
					else {
						temp_coroutine->set_state(coroutine::state::pause);}
			  }

				//没有找到需要执行的任务
				else {
					if (idle_coroutine->get_state() == coroutine::state::terminate) {
						break;
						//如果没有任务处理，并且该线程等候时间已经超时（由定时器控制）,退出循环
					}
					m_freethreadnum++;
					idle_coroutine->swap_enter();//切换到空闲等候处理的协程
					m_freethreadnum--;
					if (idle_coroutine->get_state() == coroutine::state::ready) {
						dispatch(idle_coroutine);
						idle_coroutine.reset();
					}
					else if (idle_coroutine->get_state() == coroutine::state::terminate
						|| idle_coroutine->get_state() == coroutine::state::error) {
						idle_coroutine.reset();
					}
					else {
						idle_coroutine->set_state(coroutine::state::pause);
						idle_coroutine.reset();
					}
				}
			
		}//跳出循环
	}
	//对任务的处理的基础流程是，先判断初始的状态，找到执行任务，根据运行后的状态来操作

	void schedule::stop() {
		m_autostop = true;
		if (m_root_threadid != -1)
			assert(get_this_schedule() == this);
		//采用主调度器管理
		if (m_root_threadid == -1)
			assert(get_this_schedule() != this);
		//不采用主调度器管理
		if (m_root_coroutine
			&& (m_root_coroutine->get_state() == coroutine::terminate
				|| m_root_coroutine->get_state() == coroutine::error)) {
			m_running = false;
			if (can_stop())
				return;
		}
		if (m_root_coroutine) {
			if (!can_stop())
				m_root_coroutine->call();
		}
		m_running = false;
		for (size_t i = 0; i < m_threadsnum; i++) {
			notify();
			m_threads[i]->join();//等待所有线程执行完毕
		}
		if (can_stop())
			return;
	}

	bool schedule::can_stop() {
		locallock<Mutex>lock(*m_mutex);
		if (m_autostop && !m_activethreadnum && m_coroutines.empty() && !m_running)
			return true;
		return false;
	}

	void schedule::notify(){
	}

	void schedule::idle(){
		
	}
