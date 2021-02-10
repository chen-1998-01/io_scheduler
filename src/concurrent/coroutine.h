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

	class coroutine :public std::enable_shared_from_this<coroutine> {
		friend class schedule;
	private:
		coroutine();//默认构造函数用于静态加载
	public:
		typedef std::shared_ptr<coroutine>coroutine_ptr;
		coroutine(std::function<void()> func, size_t  stack_size,bool mainfiber);
		//func 为回调函数，stack_size为开辟的栈的大小，mainfiber用来标记是否采用主协程进行管理
		~coroutine();
		enum state {
			init=0,
			ready=1,
			running=2,
			pause=3,
			terminate=4,
			error=5
		};
		void swap(std::function<void()>);
		void swap_enter();
		void swap_out();
		void call();
		void back();
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
	static void mainthread_function();
	static void schedule_function();
	public:
		static std::shared_ptr<coroutine> get_this_coroutine();
		static void set_coroutine(coroutine*);
		static uint32_t get_this_coroutineid();
		static uint32_t get_coroutinenum();
		static void yieldtoready();
		static void yieldtopause();
	private:
		ucontext_t m_context;
		uint32_t m_id;
		void* m_stack;
		uint32_t m_stack_size;
		std::function<void()>m_function;
		state m_state;
	};


#endif