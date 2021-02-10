#ifndef _THREAD_H_  
#define _THREAD_H_

#include<iostream>
#include<functional>
#include<memory>
#include<pthread.h>
#include"lock.h"
#include<unistd.h>
#include<sys/syscall.h>

static pid_t get_thread_id(){
	return syscall(SYS_gettid);
}

	class Thread {
	public:
	  enum state{
       init=0,
			 ready=1,
			 running=2,
       blocked=3,
			 free=4,
			 terminate=5
		};	
	private:
		pthread_t m_thread;
		pid_t m_id;
		std::string m_name;
		std::function<void()>m_function;//启动函数
	  Semaphore* m_semaphore;//信号量，启动回调函数时，信号量减一，回调函数运行结束后，信号量加一
	  state m_state;
	public:
		typedef std::shared_ptr<Thread>thread_ptr;
		Thread(std::function<void()>,const std::string&);
		void join();
		static void* run(void* arg);//线程入口函数，通过线程入口函数来设置当前运行线程为该线程，启动线程绑定的函数
		static pid_t get_this_threadid();
		pid_t get_id()const{
       return m_id;
		}
		~Thread();
		state get_state()const{
			return m_state;
		}
		void set_state(state _state){
			m_state=_state;
		}
	public:
		static void set_name(const std::string&);
		static Thread* get_this_thread();
		static std::string& get_this_threadname();
	private:
		Thread(const Thread&) = delete;
		Thread(Thread&&) = delete;
		Thread& operator=(Thread&) = delete;
	};



#endif
