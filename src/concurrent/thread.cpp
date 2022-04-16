#include"../../include/thread.h"
#include"../../include/coroutine.h"
#include"../../include/logger.h"



static thread_local Thread* g_thread = nullptr;//当前运行的线程
static thread_local std::string g_name = "unknown";
static  thread_local logger::_logger g_logger=root_logger;




Semaphore::Semaphore(const int& count):
    m_count(0){};

 Semaphore::~Semaphore(){
      m_count=0;  	  
  }

void Semaphore::wait(){
	  std::unique_lock<std::mutex>lock(m_mutex);
	  m_condition.wait(lock,[&](){return m_count>0;});
	  m_count--;
}

void Semaphore::release(){
	   m_count++;
	   m_condition.notify_one();
}



Thread::Thread(std::function<void()>func,const std::string& name) :
	m_thread(0), m_id(0), m_name("unknow"),m_function(func),m_state(state::init){
		if (!name.empty())
			m_name = name;
		else
		  m_name="unknown";
		m_semaphore = new Semaphore(0);
		m_state=state::ready;
		int value = pthread_create(&m_thread, NULL, &Thread::run,this);
		//创建线程，tid_t表示新线程id，attr表示新线程的属性，start_rtn表示执行函数，arg表示执行函数的传递参数
		m_semaphore->wait();//信号量减一
}

void Thread::join() {
		if(m_thread){	
	    	int value=pthread_join(m_thread,NULL);		
		}
		m_thread=0;
}

void* Thread::run(void* arg) {
		Thread* newthread = (Thread*)arg;//非静态变量拷贝到局部变量，交给静态方法调用
		g_thread = newthread;
		g_name = newthread->m_name;
		newthread->m_id =get_thread_id();
        pthread_setname_np(pthread_self(),newthread->m_name.c_str());
	    Semaphore* m_semaphore = newthread->m_semaphore;
	    m_semaphore->release();//信号量加一
	    std::function<void()>func;
	    func.swap(newthread->m_function);
		newthread->set_state(state::running);
		logger_inform_eventstream(g_logger)<<"thread running...";
		ConsoleLog();
		func();
	    return 0;
}

pid_t Thread::get_thread_id(){
		return syscall(SYS_gettid);
}

Thread* Thread::get_this_thread(){
			return (g_thread);
}

std::string& Thread::get_this_threadname(){
            return (g_name);
}
	

void Thread::set_name(const std::string& name){
	            g_name=name;      
}

Thread::~Thread() {
    if(m_thread)
		  pthread_detach(m_thread);
    m_state=state::terminate;
	logger_inform_eventstream(g_logger)<<"thread exit normally";
	ConsoleLog();
}
