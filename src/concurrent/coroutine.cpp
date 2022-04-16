#include"../../include/coroutine.h"
#include"../../include/schedule.h"
#include"../../include/hook.h"
#include<exception>
#include"../../include/logger.h"

static thread_local  coroutine* cur_fiber = nullptr;//表示所在线程当前运行的协程
static thread_local coroutine::coroutine_ptr init_fiber = nullptr;//表示所在线程的初始化协程(线程启动后的第一个协程)
static std::atomic<uint32_t>coroutine_id{0};
static std::atomic<uint32_t>coroutine_num{0};

static thread_local logger::_logger g_logger = root_logger;
	
	//getcontext 获取当前的上下文
	//makecontext 创建新的上下文,并且保留
	//swapcontex 切换上下文,保留当前的上下文，并且执行下一个上下文。
coroutine::coroutine():
	m_state(init),m_id(coroutine_id),m_stack(NULL),m_stack_size(1024) {
		assert(init_fiber == nullptr);
		coroutine_num++;
		set_coroutine(this);
		m_state = running;
		getcontext(&m_context);
		hook::SetHookState(true);
}


coroutine::coroutine(std::function<void()>func, size_t stack_size, bool Use_Initthread):
	m_stack_size(stack_size),m_state(init),m_id(++coroutine_id){
		hook::SetHookState(true);
		m_function=func;
		set_coroutine(this);
		coroutine_num++;
		getcontext(&m_context);
		m_context.uc_link = NULL;
		m_stack = malloc(stack_size);
		m_context.uc_stack.ss_size=m_stack_size;
		m_context.uc_stack.ss_sp = m_stack;
		if(!Use_Initthread)
		  makecontext(&m_context, &schedule_function, 0);
		else
		  makecontext(&m_context,&MainThread_function,0);   
}

coroutine::~coroutine() {
		//logger_inform_eventstream(g_logger)<<"A fiber been free ";
		//ConsoleLog();
		m_state = terminate;
		coroutine_num--;
		if(m_stack){
			free(m_stack);
			m_stack=nullptr;
		}
		if(cur_fiber==this)
          set_coroutine(nullptr);	
}

std::shared_ptr<coroutine> coroutine::get_this_coroutine() {
		if (cur_fiber == nullptr) {
			cur_fiber=new coroutine();
			init_fiber.reset(new coroutine());
		}
		return cur_fiber->shared_from_this();
}

void coroutine::set_coroutine(coroutine* fiber) {
		 cur_fiber=fiber;
}

uint32_t coroutine::get_coroutine_id() {
		if(!cur_fiber)
			return 0;
		return cur_fiber->get_id();	
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
		makecontext(&m_context, &schedule_function, 0);
}

	
void coroutine::swap_enter() {
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&schedule::get_maincoroutine()->get_context(), &m_context);
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		        ConsoleLog(); 
		}
		//logger_inform_eventstream(g_logger)<<"fiber running ...";
        //ConsoleLog();
}

void coroutine::swap_out() {
		set_coroutine(schedule::get_maincoroutine());
		int success=swapcontext(&m_context, &schedule::get_maincoroutine()->get_context());
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
	            ConsoleLog();	        
		}
}
	
void coroutine::call() {
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&init_fiber->get_context(),&m_context);
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		        ConsoleLog();
		}
}

void coroutine::back() {
		set_coroutine(init_fiber.get());
		int success=swapcontext(&m_context, &init_fiber->get_context());
		if(success){
				logger_error_eventstream(g_logger)<<"the coroutine switched failed ";
		        ConsoleLog();
		}
}
	

void coroutine::yieldtopause() {
		coroutine* _current = get_this_coroutine().get();
		_current->m_state = pause;
		_current->swap_out();
}

void coroutine::yieldtoready() {
		coroutine* _current = get_this_coroutine().get();
		_current->m_state = ready;
		_current->swap_out();
}


void coroutine::schedule_function() {
		std::shared_ptr<coroutine>_current = coroutine::get_this_coroutine();
		try {
			if(_current->m_function)
			  _current->m_function();
			_current->m_function=nullptr;
			_current->m_state = terminate;
		}
		catch (std::exception& except) {
			_current->m_state = error;
			logger_error_eventstream(g_logger)<<except.what();				
		}
		coroutine* ptr = _current.get();
		_current.reset();
		ptr->swap_out();
}

void coroutine::MainThread_function(){
	std::shared_ptr<coroutine>_current=coroutine::get_this_coroutine();
	assert(_current);
	try {
		if(_current->m_function)
		  _current->m_function();
		_current->m_function=nullptr;
		_current->m_state = terminate;
	}
	catch (std::exception& except) {
		_current->m_state = error;
		logger_error_eventstream(g_logger)<<except.what();				
	}
	coroutine* ptr = _current.get();
	_current.reset();
	ptr->back();
}	
