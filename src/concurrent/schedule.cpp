#include"../../include/logger.h"
#include"../../include/schedule.h"
#include"../../include/hook.h"

static thread_local logger::_logger g_logger = root_logger;

static 	thread_local schedule* g_cur_schedule = nullptr;
//表示当前运行的调度器
static thread_local coroutine* g_main_coroutine = nullptr;
//表示调度器上的主协程

schedule::schedule(size_t thread_num,const std::string& name,bool Use_InitThread):
	m_root_coroutine(nullptr),m_running(false),m_autostop(false) {
		assert(thread_num);
		m_name = name;
		coroutine::get_this_coroutine();//加载初始化协程
		//assert(g_cur_schedule == nullptr);
		g_cur_schedule = this;
		if (Use_InitThread) {
			Thread::set_name(m_name);//线程名称采取和调度器相同的名称
			m_root_threadid = Thread::get_thread_id();
			m_threads_id.push_back(m_root_threadid);
			m_root_coroutine.reset(new coroutine(std::bind(&schedule::run, this), (128*1024),true));
			g_main_coroutine = m_root_coroutine.get(); 
			thread_num--;
			logger_inform_eventstream(g_logger)<<"Main scheduler init...";
		    ConsoleLog();
		}
		else {
			m_root_coroutine=nullptr;
			m_root_threadid = -1;
		}
		m_threadsnum = thread_num;	
}

schedule::~schedule() {
	if(m_running)
	  stop();
	if (get_this_schedule() == this) {
	    g_cur_schedule = nullptr;			
	}  
	//logger_inform_eventstream(g_logger)<<"scheduler exit normally";
	//ConsoleLog();
}//当调度器已经停止运行时才能析构该调度器，如果该调度器恰好为主调度器，则将当期运行调度器置为空

	schedule* schedule::get_this_schedule() {
		return g_cur_schedule;
	}

	void schedule::set_this_schedule(schedule* _schedule) {
		g_cur_schedule = _schedule;
	}

	coroutine* schedule::get_maincoroutine() {
		return g_main_coroutine;
	}


	void schedule::start() {
		    std::unique_lock<std::mutex>lock(m_mutex);
			set_this_schedule(this);//设置当前的调度器
			if (m_running)
				return;	
			m_threads.resize(m_threadsnum);
			for (int i = 0; i < m_threadsnum; i++) {		        		
				m_threads[i].reset(new Thread(std::bind(&schedule::run, this), m_name));
				m_threads_id.push_back(m_threads[i]->get_id());
			}		
	}//建立线程池,开始执行任务


	void schedule::run() {
		hook::SetHookState(true);
		if(Thread::get_thread_id()!=m_root_threadid)
		   g_main_coroutine=coroutine::get_this_coroutine().get();   
		coroutine_s temp;//暂存当前任务
		coroutine::coroutine_ptr idle_coroutine(new coroutine(std::bind(&schedule::idle,this),128*1024));
		coroutine::coroutine_ptr temp_coroutine;//绑定任务并且执行
		while (true) {
			temp.reset();
			temp_coroutine=nullptr;
			bool need_notify = false;
			{
				std::unique_lock<std::mutex>lock(m_mutex);   
				std::list<schedule::coroutine_s>::iterator i = m_coroutines.begin();
				while (i != m_coroutines.end()){
					if (i->thread_id != -1 && i->thread_id != Thread::get_thread_id()) {
						need_notify = true;
						i++;
						continue;
					}//不在当前线程执行的任务
					if (i->s_coroutine && i->s_coroutine->get_state() == coroutine::state::running) {
						i++;
						continue;
					}//协程已经正在执行的任务
					assert(i->s_coroutine || i->s_function);
					temp = *i;
					m_coroutines.erase(i);
					break;
				}
			}//交给线程池来争夺任务，一旦某个线程争夺到任务，就将资源提供保护，然后遍历任务队列，找到当前线程需要执行的任务
				
				if (need_notify)
					notify();
					
				if ( temp.s_coroutine && temp.s_coroutine->get_state() != coroutine::state::terminate) {
					m_activethreadnum++;
					logger_inform_eventstream(g_logger)<<"schedule processing";
                    ConsoleLog();
					temp.s_coroutine->swap_enter();
				    //如果为协程，将其切换为当前运行的协程
					//接着判断运行结束后的状态
					m_activethreadnum--;
					if (temp.s_coroutine->get_state() == coroutine::state::ready) {
						dispatch(temp.s_coroutine);
						temp.reset();
					}//运行结束后当是ready状态时，重新放入到任务队列中
					else if (temp.s_coroutine->get_state() != coroutine::state::terminate && temp.s_coroutine->get_state() != coroutine::state::error) {
						temp.s_coroutine->set_state(coroutine::state::pause);
						temp.reset();
					}
					else {
						temp.reset();
					}
					continue;
				}

        //如果为回调函数，则将其绑定一个协程,然后加入到当前的线程任务中
				else if (temp.s_function) {
					std::function<void()>temp_function;
					temp_function.swap(temp.s_function);
					if ( temp_coroutine)
						temp_coroutine->swap(temp_function);
					else if (temp_coroutine == nullptr) {
						temp_coroutine.reset(new coroutine(temp_function, 128*1024,true));
					}
					temp.reset();
					m_activethreadnum++;
					logger_inform_eventstream(g_logger)<<"schedule processing";
                    ConsoleLog();
					temp_coroutine->swap_enter();
					m_activethreadnum--;
					if (temp_coroutine->get_state() == (coroutine::state::error | coroutine::state::terminate)) {
						temp_coroutine->swap(nullptr);
					}
					else if(temp_coroutine->get_state()==coroutine::state::ready){
						dispatch(temp_coroutine);
				    }
					else {
						temp_coroutine->set_state(coroutine::state::pause);
					}
					continue;
			  }

				//没有找到需要执行的任务
				else{   
					if (idle_coroutine->get_state() == coroutine::state::terminate) {
						break;
					}
					m_freethreadnum++;
					idle_coroutine->swap_enter();//切换到空闲等候处理的协程
					m_freethreadnum--; 
				    if(idle_coroutine->get_state()!=coroutine::state::terminate && 
					   idle_coroutine->get_state()!=coroutine::state::error){
						   idle_coroutine->set_state(coroutine::state::pause);
					   }
				}
			}
	}
	//对任务的处理的基础流程是，先判断初始的状态，找到执行任务，根据运行后的状态来操作

bool schedule::has_freethread()const{
	if(!m_freethreadnum)
	  return false;
	return true;  
}



void schedule::stop() {
		m_autostop = true;
		if (m_root_threadid != -1)
			assert(get_this_schedule() == this);
		if (m_root_coroutine
		   && (m_root_coroutine->get_state() == coroutine::terminate
		   || m_root_coroutine->get_state() == coroutine::error)) {
			m_running = false;
			if (can_stop())
				return;
		}
		if (m_root_coroutine) 
			m_root_coroutine->call();
		
		m_running = false;
			for(size_t i = 0; i < m_threadsnum; i++){
			    notify();
		    }
		    for (size_t i = 0; i < m_threadsnum; i++) {
			    m_threads[i]->join();//等待所有线程执行完毕
		    }
		if(can_stop()){
		  logger_inform_eventstream(g_logger)<<"scheduler stopping...";
		  ConsoleLog();
		 } 
	}

	bool schedule::can_stop(){ 
		return false;
	}

	void schedule::notify(){
		return;
	}

	void schedule::idle(){
		return;
	}
    