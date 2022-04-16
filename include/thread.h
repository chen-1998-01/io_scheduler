#ifndef _THREAD_H_  
#define _THREAD_H_

#include<iostream>
#include<functional>
#include<memory>
#include<pthread.h>
#include<mutex>
#include<condition_variable>
#include<unistd.h>
#include<sys/syscall.h>


class Semaphore{
    private:
		int m_count;
        std::mutex m_mutex;
		std::condition_variable m_condition; 
	public:
	    Semaphore(const int& count=0);
		~Semaphore();
		void wait();
		void release();	    
};

class Thread {
   public:
	  enum state{
             init=0,
			 ready=1,
			 running=2,
             blocked=3,
			 free=4,
			 terminate=5
		};//线程的状态	
	private:
		pthread_t m_thread;
		pid_t m_id;
		std::string m_name;
		std::function<void()>m_function;//启动函数
	    Semaphore* m_semaphore;
	    state m_state;
	public:
		typedef std::shared_ptr<Thread>thread_ptr;
		Thread(std::function<void()>,const std::string&);
		void join();
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
	    static void* run(void* arg);
	    //线程入口函数，用来切换线程的，通过线程入口函数来设置当前运行线程为该线程，启动线程绑定的函数
		static void set_name(const std::string&);
		static Thread* get_this_thread();
		static std::string& get_this_threadname();
		static pid_t get_thread_id();
	private:
		Thread(const Thread&) = delete;
		Thread(Thread&&) = delete;
		Thread& operator=(Thread&) = delete;
	};


#endif
