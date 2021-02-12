#include"coroutine.h"
#include"schedule.h"
#include<exception>
#include"../manager/logger.h"


	static thread_local coroutine* g_cur_fiber = nullptr;//表示当前运行的协程
	static thread_local std::shared_ptr<coroutine> g_init_fiber = nullptr;//表示用于初始化的协程
	static std::atomic<uint32_t>coroutine_id{0};
	static std::atomic<uint32_t>coroutine_num{0};

  static thread_local logger::_logger g_logger=root_logger();

	class malloc_allocator { 
	public:
		static void* allocate(size_t size) {
			return malloc(size);
		}
		static void deallocate(void* memory,size_t size) {
			free(memory);
			size = 0;
		}

	};

	//getcontext 初始化上下文
	//makecontext 保存上下文,执行协程回调函数
	//swapcontex 切换上下文
	coroutine::coroutine():
	m_state(init),m_id(++coroutine_id),m_stack(NULL),m_stack_size(0) {
		assert(g_init_fiber == nullptr);
		coroutine_num++;
		set_coroutine(this);
		m_state = running;
		getcontext(&m_context);
		logger_inform_eventstream(g_logger)<<"initializational fiber been created ";
		record(g_logger,logger_level::inform);
	}//创建默认的初始化协程，不绑定任务函数


	coroutine::coroutine(std::function<void()>func, size_t stack_size,bool mainfiber):
	m_stack_size(stack_size),m_state(init),m_function(func),m_id(++coroutine_id){
		set_coroutine(this);
		coroutine_num++;
		getcontext(&m_context);
		m_context.uc_link = NULL;
		m_stack = malloc_allocator::allocate(stack_size);
		m_context.uc_stack.ss_size=m_stack_size;
		m_context.uc_stack.ss_sp = m_stack;
		if (mainfiber){
			makecontext(&m_context, &mainthread_function, 0);
		//如果是调度器主协程，执行完之后，回到初始化线程
		}
		else {
			makecontext(&m_context, &schedule_function, 0);
		}
		//如果不是调度器主协程，执行完之后，回到调度器主协程
	}

	coroutine::~coroutine() {
		logger_inform_eventstream(g_logger)<<"A fiber been free ";
		record(g_logger,logger_level::inform);
		m_state = terminate;
		coroutine_num--;
		if(m_stack)
		  malloc_allocator::deallocate((void*)m_stack, m_stack_size);
		else{
			if(g_cur_fiber==this)
        set_coroutine(nullptr);
		}	
	}

	std::shared_ptr<coroutine> coroutine::get_this_coroutine() {
		if (g_cur_fiber == nullptr) {
			g_cur_fiber=new coroutine();
			g_init_fiber.reset(g_cur_fiber);
		}
		return g_cur_fiber->shared_from_this();
	}//获取当前运行的协程


	void coroutine::set_coroutine(coroutine* fiber) {
		 g_cur_fiber=fiber;
	}

	uint32_t coroutine::get_this_coroutineid() {
		  if(!g_cur_fiber)
			  return 0;
			return g_cur_fiber->get_id();	
	}

	uint32_t coroutine::get_coroutinenum() {
		return coroutine_num;
	}


	void coroutine::swap(std::function<void()> func) {
		assert(m_stack);
		assert(m_state!=running);
		m_function.swap(func);
		getcontext(&m_context);
		m_context.uc_link = NULL;
		m_context.uc_stack.ss_size = m_stack_size;
		m_context.uc_stack.ss_sp = m_stack;
		makecontext(&m_context, &mainthread_function, 0);
	}//切换协程上的回调函数

	
	void coroutine::swap_enter() {
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&schedule::get_currentcoroutine()->get_context(), &m_context);
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		    record(g_logger,logger_level::inform);
		}

	}

	void coroutine::swap_out() {
		set_coroutine(schedule::get_currentcoroutine());
		int success=swapcontext(&m_context, &schedule::get_currentcoroutine()->get_context());
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		    record(g_logger,logger_level::inform);
		}

	}
  //与调度器主协程之间进行切换
	
	void coroutine::call() {
		assert(m_state!=running);
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&g_cur_fiber->get_context(),&m_context);
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		    record(g_logger,logger_level::inform);
		}

	}

	void coroutine::back() {
		assert(m_state == running);
		set_coroutine(g_init_fiber.get());
		int success=swapcontext(&m_context, &g_init_fiber->get_context());
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		    record(g_logger,logger_level::inform);
		}

	}
	//与初始化协程之间进行切换

	void coroutine::yieldtopause() {
		std::shared_ptr<coroutine>_current = get_this_coroutine();
		_current->m_state = pause;
		_current->swap_out();
	}

	void coroutine::yieldtoready() {
		std::shared_ptr<coroutine>_current = get_this_coroutine();
		_current->m_state = ready;
		_current->swap_out();
	}
  //挂起当前的协程

	void coroutine::mainthread_function() {
		std::shared_ptr<coroutine>_current =coroutine::get_this_coroutine();
		try {
			logger_inform_eventstream(g_logger)<<"A fiber been runned ";
		  record(g_logger,logger_level::inform);
			_current->m_function();
			_current->m_function = NULL;
			_current->m_state = terminate;
		}
		catch (std::exception& except) {
			_current->m_state = error;
			logger_error_eventstream(g_logger)<<"the coroutine running error ";
		  record(g_logger,logger_level::inform);			
		}
		coroutine* ptr = _current.get();
		_current.reset();
		ptr->back();//跳回到初始化协程中
	}

	void coroutine::schedule_function() {
		std::shared_ptr<coroutine>_current = coroutine::get_this_coroutine();
		try {
			logger_inform_eventstream(g_logger)<<"A fiber been runned ";
		  record(g_logger,logger_level::inform);
			_current->m_function();
			_current->m_function = NULL;
			_current->m_state = terminate;
		}
		catch (std::exception& except) {
			_current->m_state = error;
			logger_error_eventstream(g_logger)<<"the coroutine running error ";
		  record(g_logger,logger_level::inform);				
		}
		coroutine* ptr = _current.get();
		_current.reset();
		ptr->swap_out();//跳回到调度器主协程中
	}

	




