#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include"thread.h"
#include"lock.h"
#include"coroutine.h"
#include<vector>
#include<list>
#include<atomic>

	//调度器设计
	//通过线程池调度协程

	class schedule {
	public:
		typedef std::shared_ptr<schedule>schedule_ptr;
		schedule(size_t thread_num, bool use_mainfiber, const std::string& name);
		virtual ~schedule();
	private:
		std::string m_name;
		Mutex m_mutex;
		std::vector<Thread::thread_ptr>m_threads;
		size_t m_threadsnum;
		std::atomic<uint32_t>m_freethreadnum{ 0 };
		std::atomic<uint32_t>m_activethreadnum{ 0 };
		//使用原子操作，避免线程安全问题
		bool m_running;//运行状态
		bool m_autostop;//能否自动停止运行
		std::vector<int>m_threads_id;
		uint32_t m_root_threadid;
		coroutine::coroutine_ptr m_root_coroutine;//主协程用于管理
	public:
		static coroutine* get_maincoroutine();
		static schedule* get_this_schedule();
		static void set_this_schedule(schedule*);
		//static std::string get_this_name();
		void start();//启动
		void run();
		void stop();//停止
	protected:
		bool can_stop();//判断是否具有停止的条件
	    virtual	void notify();//消息通知
		virtual void idle();//空闲处理
	private:
		//任务协程结构
		struct coroutines {
			coroutine::coroutine_ptr coroutine;
			std::function<void()>function;
			int thread_id;
			coroutines():coroutine(nullptr),function(nullptr),thread_id(0){};
			coroutines(int _thread_id, std::function<void()>_function) :thread_id(_thread_id),
				function(_function) {};
			coroutines(int _thread_id, std::function<void()>*_function) :thread_id(_thread_id) {
				function.swap(*_function);
			}
			coroutines(int _thread_id, coroutine::coroutine_ptr fiber) :thread_id(_thread_id),
			coroutine(fiber){};
			coroutines(int _thread_id, coroutine::coroutine_ptr *fiber) :thread_id(_thread_id) {
				coroutine.swap(*fiber);
			}
			void reset() {
				thread_id = -1;
				function = NULL;
				coroutine.reset();
			}
		};
	private:
		std::list<schedule::coroutines>m_coroutines;
		//存储执行的任务
		template<typename callback_fiber>
		bool m_dispatch(int thread_id, callback_fiber function) {
			bool need_call = m_coroutines.empty();
			inet::schedule::coroutines task(thread_id, function);
			if (task.m_coroutine || task.m_function)
				m_coroutines.push_back(task);
			return need_call;
		}
	public:
		template<typename callback_fiber>
		void dispatch(int thread_id, callback_fiber function) {
			bool need_call = false;
			{
				inet::locallock<inet::Mutex>lock(m_mutex);
				need_call = m_dispatch(thread_id, function);
			}
			if (need_call) {
				notify();
			}
		}

		template<typename callback_fiber>
		void dispatch(callback_fiber function) {
			bool need_call = false;
			{
				inet::locallock<inet::Mutex>lock(m_mutex);
				need_call = m_dispatch(-1, function);
			}
			if (need_call)
				notify();
		}//一次添加一个任务

		template<typename callback_fiber_iterator>
		void dispatch(int thread_id,callback_fiber_iterator *begin, callback_fiber_iterator *end) {
			bool need_call = false;
			{
				inet::locallock<inet::Mutex>lock(m_mutex);
				while (*begin != *end)
				{
					dispatch(thread_id,*(*begin));
					(*begin)++;
					if (need_call) {
						notify();
					}
				}
			}
		}//采用迭代器一次性添加批量任务，参数传递为迭代器
	};

#endif
