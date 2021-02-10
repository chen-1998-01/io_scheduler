#include"lock.h"
#include<cassert>

	Semaphore::Semaphore(uint32_t _num) {
		int value=sem_init(&m_semphore, 0, _num);
		if (value)
			std::cout << "semaphore init failed" << std::endl;
	}

	void Semaphore::wait() {
		if (sem_wait(&m_semphore))
			std::cout << "semphore wait failed" << std::endl;
	}

	void Semaphore::release() {
		if(sem_post(&m_semphore))
			std::cout<<"semphore post failed"<<std::endl;
	}

	Semaphore::~Semaphore(){
		sem_destroy(&m_semphore);
	}
	Mutex::Mutex() {
		int init_value=pthread_mutex_init(&m_mutex, NULL);
	}

	Mutex::~Mutex() {
		pthread_mutex_destroy(&m_mutex);
	}

	void Mutex::trylock() {
		int locked=pthread_mutex_trylock(&m_mutex);
	}

	void Mutex::lock() {
		pthread_mutex_lock(&m_mutex);
	}

	void Mutex::unlock() {
		pthread_mutex_unlock(&m_mutex);
	}

	condition_variable::condition_variable(Mutex& mutex):
	m_condition_num(0){
		pthread_mutex_t* address =&(mutex.m_mutex);
		assert(address != NULL);
		m_mutex =address;
	}

	condition_variable::~condition_variable() {
		for(int i=0;i<m_conditions.size();i++){
			pthread_cond_destroy(&(std::get<2>(m_conditions[i])));
		}
		m_conditions.clear();
		m_condition_num=0;
	}

	void condition_variable::add_conductor(const std::string& condition) {
		pthread_cond_t conductor;
		int value=pthread_cond_init(&conductor,NULL);
		std::tuple<int,std::string,pthread_cond_t> newtuple=std::make_tuple(g_conductor,condition,conductor);
		m_conditions.push_back(newtuple);
		m_condition_num++;
	}

	void condition_variable::add_consumer(const std::string& condition) {
		pthread_cond_t consumer;
		int value = pthread_cond_init(&consumer,NULL);
		std::tuple<int,std::string,pthread_cond_t> newtuple=std::make_tuple(g_conductor,condition,consumer);
		m_conditions.push_back(newtuple);
		m_condition_num++;
	}

	void condition_variable::wait_consume(const std::string& condition) {
		if(m_conditions.empty())
		  return;
		for(int i=0;i<m_conditions.size();i++){
        if(std::get<0>(m_conditions[i])==g_conductor && std::get<1>(m_conditions[i])==condition){
					pthread_cond_wait(&(std::get<2>(m_conditions[i])),m_mutex);
					break;
				}
				 
		}
	}

	void condition_variable::wait_conduct(const std::string& condition) {
		if(m_conditions.empty())
		  return;
		for(int i=0;i<m_conditions.size();i++){
			 if(std::get<0>(m_conditions[i])==g_consumer && std::get<1>(m_conditions[i])==condition){
      	  pthread_cond_wait(&(std::get<2>(m_conditions[i])),m_mutex);
					break;
			 }
		}
	}

	void condition_variable::notify_one(int tag,const std::string& condition){
		if (tag == g_conductor) {
			for(int i=0;i<m_conditions.size();i++){
         if(std::get<0>(m_conditions[i])==g_conductor && std::get<1>(m_conditions[i])==condition){
            pthread_cond_signal(&(std::get<2>(m_conditions[i])));
						break;
				 }
			}
		}
		else if (tag == g_consumer) {			
			for(int i=0;i<m_conditions.size();i++){
         if(std::get<0>(m_conditions[i])==g_consumer && std::get<1>(m_conditions[i])==condition){
            pthread_cond_signal(&(std::get<2>(m_conditions[i])));
						break;
				 }
			}
		}
	}

	void condition_variable::notify_all(int tag,const std::string& condition) {
	if (tag == g_conductor) {
			for(int i=0;i<m_conditions.size();i++){
         if(std::get<0>(m_conditions[i])==g_conductor && std::get<1>(m_conditions[i])==condition){
            pthread_cond_broadcast(&(std::get<2>(m_conditions[i])));
						break;
				 }
			}
		}
		else if (tag == g_consumer) {			
			for(int i=0;i<m_conditions.size();i++){
         if(std::get<0>(m_conditions[i])==g_consumer && std::get<1>(m_conditions[i])==condition){
            pthread_cond_broadcast(&(std::get<2>(m_conditions[i])));
						break;
				 }
			}
		}		
	}

	SpinMutex::SpinMutex() {
		int init_value=pthread_spin_init(&m_mutex, 0);
	}

	SpinMutex::~SpinMutex() {
		pthread_spin_destroy(&m_mutex);
	}

	void SpinMutex::trylock() {
		int locked=pthread_spin_trylock(&m_mutex);
	}

	void SpinMutex::lock() {
		pthread_spin_lock(&m_mutex);
	}

	void SpinMutex::unlock() {
		pthread_spin_unlock(&m_mutex);
	}

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

	RW_Mutex::RW_Mutex() {
		int init_value=pthread_rwlock_init(&m_mutex, NULL);
	}

	RW_Mutex::~RW_Mutex() {
		pthread_rwlock_destroy(&m_mutex);
	}


	void RW_Mutex::write_trylock() {
		int locked = pthread_rwlock_trywrlock(&m_mutex);
	}

	void RW_Mutex::write_lock() {
		pthread_rwlock_wrlock(&m_mutex);
	}

	void RW_Mutex::write_unlock() {
		pthread_rwlock_unlock(&m_mutex);
	}

	void RW_Mutex::read_trylock() {
		int locked = pthread_rwlock_tryrdlock(&m_mutex);
	}

	void RW_Mutex::read_lock() {
		pthread_rwlock_rdlock(&m_mutex);
	}

	void RW_Mutex::read_unlock() {
		pthread_rwlock_unlock(&m_mutex);
	}

	template<class T>
	localwritelock<T>::localwritelock(const T&_mutex) {
		m_mutex = _mutex;
		m_mutex.write_lock();
		lock_state = true;
	}

	template<class T>
	localwritelock<T>::~localwritelock() {
		if (lock_state)
			m_mutex.write_unlock();
		lock_state = false;
	}

	template<class T>
	localreadlock<T>::localreadlock(const T&_mutex) {
		m_mutex = _mutex;
		m_mutex.read_lock();
		lock_state = true;
	}

	template<class T>
	localreadlock<T>::~localreadlock() {
		if (lock_state)
		    m_mutex.read_unlock();
		lock_state = false;
	}

	template<class T>
	void localwritelock<T>::write_lock() {
		   m_mutex.write_lock();
       }

	template<class T>
	void localwritelock<T>::write_unlock() {
		   m_mutex.write_unlock();
	   }

	template<class T>
	void localreadlock<T>::read_lock() {
		   m_mutex.read_lock();
	   }

	template<class T>
	void localreadlock<T>::read_unlock() {
		   m_mutex.read_unlock();
	   }
