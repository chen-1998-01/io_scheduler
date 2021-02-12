#ifndef _LOCK_H_
#define _LOCK_H_
#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<stdint.h>
#include<vector>
#include<map>
#include<tuple>
 
	class Semaphore {
	private:
		Semaphore(const Semaphore&) = delete;
		Semaphore(const Semaphore&&) = delete;
		Semaphore operator=(const Semaphore&) = delete;
	public:
		Semaphore(uint32_t);
		~Semaphore();
		void wait();
		void release();
	private:
		sem_t m_semphore;
	};//信号量

	class Mutex {
		friend class condition_variable;
	public:
		Mutex();
		~Mutex();
		void lock();
		void trylock();
		void unlock();
	private:
		pthread_mutex_t m_mutex;
	};//互斥量


#ifndef g_conductor
#define g_conductor 0
#endif

#ifndef g_consumer
#define g_consumer 1
#endif
//条件变量宏定义

	class condition_variable{
	private:
		pthread_mutex_t* m_mutex;
		std::vector<std::tuple<int,std::string,pthread_cond_t>>m_conditions;
		int m_condition_num;
	public:
		condition_variable(Mutex& mutex);
		~condition_variable();
		void add_conductor(const std::string&);//增加生产者条件变量
		void add_consumer(const std::string&);//增加消费者条件变量
		//每次增加一个条件进行判断，就增加一个条件变量
		void wait_consume(const std::string&);//阻塞生产者线程
		void wait_conduct(const std::string&);//阻塞消费者线程
		void notify_one(int,const std::string&);//唤醒一条被阻塞的线程
		void notify_all(int,const std::string&);//唤醒所有被阻塞的线程
		//唤醒线程，第一个参数标识条件变量属性，第二个参数标识条件变量的序号
	};//条件变量

	class SpinMutex {
	private:
		pthread_spinlock_t m_mutex;
	public:
		SpinMutex();
		~SpinMutex();
		void lock();
		void trylock();
		void unlock();
	};//自旋锁


	class RW_Mutex {
	private:
		RW_Mutex(const RW_Mutex&) = delete;
		RW_Mutex(const RW_Mutex&&) = delete;
		RW_Mutex operator=(const RW_Mutex&) = delete;
	public:
		RW_Mutex();
		~RW_Mutex();
		void write_trylock();
		void write_lock();
		void write_unlock();
		void read_trylock();
		void read_lock();
		void read_unlock();
	private:
		pthread_rwlock_t m_mutex;

	};//读写锁

	template<class T>
	class locallock {
	private:
		T m_mutex;
		bool lock_state;
	public:
		locallock(const T&_mutex);
		~locallock();
	protected:
		void lock();
		void unlock();

	};

	template<class T>
	locallock<T>::locallock(const T&_mutex) {
		m_mutex = _mutex;
		lock();
		lock_state = true;
	}

	template<class T>
	locallock<T>::~locallock() {
		unlock();
		lock_state = false;
	}

	template<class T>
	void locallock<T>::lock() {
		m_mutex.lock();
	}

	template<class T>
	void locallock<T>::unlock() {
		m_mutex.unlock();
	}


	template<class T>
	class localreadlock {
	private:
		T m_mutex;
		bool lock_state;
	public:
		localreadlock(const T&_mutex);
		~localreadlock();
	protected:
		void read_lock();
		void read_unlock();

	};

	template<class T>
	class localwritelock {
	private:
		T m_mutex;
		bool lock_state;
	public:
		localwritelock(const T&_mutex);
		~localwritelock();
	protected:
		void write_lock();
		void write_unlock();

	};

	typedef localwritelock<RW_Mutex> unshared_lock;
	typedef localreadlock<RW_Mutex>shared_lock;

#endif 

