#ifndef _COROUTINE_H_
#define _COROUTINE_H_
#include<ucontext.h>
#include<memory>
#include<functional>
#include<iostream>
#include<cstdio>
#include<cstdint>
#include<ctime>
#include<cstdlib>
#include<atomic>
#include<cassert>
#include"thread.h"


class schedule;//调度器

class coroutine:public std::enable_shared_from_this<coroutine> {
	friend class schedule;
	public:	enum state {
			init=0,
			ready=1,
			running=2,
			pause=3,
			terminate=4,
			error=5
		};
	private:
		ucontext_t m_context;
		uint32_t m_id;
		void* m_stack;
		uint32_t m_stack_size;
		std::function<void()>m_function;
		state m_state;
	protected:
		coroutine();//创建默认的初始化协程，不绑定任务函数
	public:
		typedef std::shared_ptr<coroutine> coroutine_ptr;
		coroutine(std::function<void()> func, size_t  stack_size, bool Use_Initthread=false);
		~coroutine();
		void swap(std::function<void()>);//切换协程执行的任务
		void swap_enter();
		void swap_out();
		//与调度器当前运行的协程进行切换
		void call();
		void back();
        //与所在线程的初始化协程进行切换

		uint32_t get_id()const {
			return m_id;
		}
		state get_state()const {
			return m_state;
		}
		void set_state(state _state) {
			m_state= _state;
		}
	  ucontext_t& get_context() { return m_context; }
	  static void schedule_function();//在调度器上面调度
	  static void MainThread_function();//在主线程上面调度
	public:
		static std::shared_ptr<coroutine> get_this_coroutine();
		static void set_coroutine(coroutine*);
		static uint32_t get_coroutine_id();
		static uint32_t get_coroutinenum();
		static void yieldtoready();
		static void yieldtopause();
	};


#endif