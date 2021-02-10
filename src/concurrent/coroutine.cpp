#include"coroutine.h"
#include"schedule.h"
#include<exception>


	static thread_local coroutine* t_coroutine = nullptr;//表示当前运行的协程
	static thread_local std::shared_ptr<coroutine> main_thread = nullptr;//表示用于初始化的协程
	static std::atomic<uint32_t>coroutine_id{ 0 };
	static std::atomic<uint32_t>coroutine_num{ 0 };

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
	coroutine::coroutine():m_state(init),m_id(++coroutine_id),m_stack(NULL),m_stack_size(0) {
		assert(main_thread == nullptr);
		coroutine_num++;
		set_coroutine(this);
		m_state = running;
		getcontext(&m_context);
	}//创建主线程

	coroutine::coroutine(std::function<void()>func, size_t stack_size,bool mainfiber):
	m_stack_size(stack_size),m_state(init),m_function(func),m_id(++coroutine_id){
		assert(main_thread != nullptr);
		set_coroutine(this);
		coroutine_num++;
		getcontext(&m_context);
		m_context.uc_link = NULL;
		m_stack = malloc_allocator::allocate(stack_size);
		m_context.uc_stack.ss_size=m_stack_size;
		m_context.uc_stack.ss_sp = m_stack;
		if (mainfiber)
			makecontext(&m_context, &mainthread_function, 0);
		//如果是调度器主协程，执行完之后，回到主线程
		else {
			makecontext(&m_context, &schedule_function, 0);
		}
		//如果不是调度器主协程，执行完之后，回到调度器主协程
	}

	coroutine::~coroutine() {
		assert(m_state != running);
		m_state = terminate;
		coroutine_num--;
		m_id = 0;
		m_function = NULL;
		malloc_allocator::deallocate((void*)m_stack, m_stack_size);
	}

	std::shared_ptr<coroutine> coroutine::get_this_coroutine() {
		if (t_coroutine == nullptr) {
			t_coroutine = new coroutine();
			main_thread = t_coroutine->shared_from_this();
		}
		return t_coroutine->shared_from_this();
	}//获取当前运行的协程


	void coroutine::set_coroutine(coroutine* fiber) {
		t_coroutine = fiber;
	}

	uint32_t coroutine::get_this_coroutineid() {
		return t_coroutine->get_id();
	}

	uint32_t coroutine::get_coroutinenum() {
		return coroutine_num;
	}


	void coroutine::swap(std::function<void()>func) {
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
		assert(m_state == init || m_state == terminate || m_state == error);
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&schedule::get_maincoroutine()->get_context(), &m_context);
	}//将该协程设置为调度器当前运行的协程

	void coroutine::swap_out() {
		assert(m_state == running);
		set_coroutine(schedule::get_maincoroutine());
		m_state = pause;
		int success=swapcontext(&m_context, &schedule::get_maincoroutine()->get_context());
	}

	
	void coroutine::call() {
		assert(m_state!=running);
		set_coroutine(this);
		m_state = running;
		int success=swapcontext(&main_thread->get_context(),&m_context);
	}//从主协程切换到该协程

	void coroutine::back() {
		assert(m_state == running);
		set_coroutine(main_thread.get());
		int success=swapcontext(&m_context, &main_thread->get_context());
	}//将该协程放在后台运行，切换到主协程

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

	void coroutine::mainthread_function() {
		std::shared_ptr<coroutine>_current = get_this_coroutine();
		try {
			_current->m_function();
			_current->m_function = NULL;
			_current->m_state = terminate;
		}
		catch (std::exception& except) {
			_current->m_state = error;			
		}
		coroutine* ptr = _current.get();
		_current.reset();
		ptr->back();
	}

	void coroutine::schedule_function() {
		std::shared_ptr<coroutine>_current = get_this_coroutine();
		try {
			_current->m_function();
			_current->m_function = NULL;
			_current->m_state = terminate;
		}
		catch (std::exception& except) {
			_current->m_state = error;			
		}
		coroutine* ptr = _current.get();
		_current.reset();
		ptr->swap_out();
	}

	




