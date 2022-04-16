#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

//调度器主要设计两种，一种负责本地磁盘调度，一种负责网络io调度
//调度器基类的设计：1.调度器中内含有线程池，可以切换线程和协程.
                 //协程的本质是函数加运行状态，可以由应用程序来切换，并且可以挂起，之后可以恢复重新执行
                 //但是协程必须要绑定线程来实现并发  
//调度器可能需要多个，所以需要设置一个主调度器进行管理                    


#include"coroutine.h"
#include<vector>
#include<list>
#include<atomic>


class schedule:std::enable_shared_from_this<schedule>{
	public:
		typedef std::shared_ptr<schedule>schedule_ptr;
		//schedule(){};
		schedule(size_t thread_num, const std::string& name,bool Use_InitThread=false);
		virtual ~schedule();
	protected:
		std::string m_name;//调度器名称
		std::mutex m_mutex;
		std::vector<Thread::thread_ptr>m_threads;//内部线程池
		size_t m_threadsnum;//新创建的线程数
		std::atomic<uint32_t>m_freethreadnum{ 0 };//空闲等待的线程数量
		std::atomic<uint32_t>m_activethreadnum{ 0 };//正在活跃的线程数
		bool m_running;//运行状态
		bool m_autostop;//能否自动停止运行
		std::vector<int>m_threads_id;
		uint32_t m_root_threadid;
		coroutine::coroutine_ptr m_root_coroutine;//调度器主协程		
	public:
		static schedule* get_this_schedule();//获取当前调度器
		static coroutine* get_maincoroutine();//获取调度器当前运行协程
		static void set_this_schedule(schedule*);//设置当前运行的调度器
		void start();//启动
		void run();//运行
		void stop();//停止
		bool has_freethread()const;
	protected:
	    virtual bool can_stop();//判断是否具有停止的条件
	    virtual	void notify();//消息通知，实现同步操作
		virtual void idle();//空闲处理

	private:
		struct coroutine_s {
			std::shared_ptr<coroutine> s_coroutine;
			std::function<void()>s_function;
			int thread_id;//执行任务的线程id,当为-1时可以由任意线程来执行
			coroutine_s():s_coroutine(nullptr),s_function(nullptr),thread_id(-1){};
			coroutine_s(int _thread_id, std::function<void()>_function) :
			  thread_id(_thread_id),s_function(_function) {}; 
			coroutine_s(int _thread_id, std::function<void()>*_function) :
			  thread_id(_thread_id) {
				s_function.swap(*_function);
			}
			coroutine_s(int _thread_id, coroutine::coroutine_ptr fiber) :
			  thread_id(_thread_id),s_coroutine(fiber){};
			coroutine_s(int _thread_id, coroutine::coroutine_ptr *fiber) :
			  thread_id(_thread_id) {
				s_coroutine.swap(*fiber);
			}
			void reset() {
				thread_id = -1;
				s_function = NULL;
				s_coroutine.reset();
			}//将任务释放
		};
		//任务接口的结构，绑定协程（协程绑定任务回调函数）
	private:	

		//分配任务
		template<typename callback_fiber>
		bool m_dispatch(callback_fiber function,int thread_id) {
			bool need_call = m_coroutines.empty();
			schedule::coroutine_s task(thread_id, function);
			if (task.s_coroutine || task.s_function)
				m_coroutines.push_back(task);
			return need_call;
		}//向任务队列插入任务 当任务队列为空时，需要进行唤醒,采用epoll触发模式
    protected:
	    std::list<schedule::coroutine_s>m_coroutines;//任务接口	
	public:
		template<typename callback_fiber>
		void dispatch(callback_fiber function,int thread_id = -1) {
			bool need_call = false;
			{
				std::unique_lock<std::mutex>lock(m_mutex);
				need_call = m_dispatch(function,thread_id);
			}
			if (need_call) {
				notify();
			}
		}
        //一次添加一个任务

		template<typename callback_fiber_iterator>
		void batch_dispatch(callback_fiber_iterator begin, callback_fiber_iterator end,int thread_id=-1) {
				while (begin != end)
				{
					dispatch(*begin,thread_id);
					begin++;
				}
		}
		//采用迭代器一次性添加批量任务
	};

#endif
