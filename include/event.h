#ifndef _EVENT_H_
#define _EVENT_H_

#include"schedule.h"
#include<pthread.h>


enum event_type{
          NONE = 0x0,
          READ = 0x1,
          WRITE = 0x4
};

class event{
friend class io_schedule;
public:
      struct event_context{
             schedule* s_scheduler;
             coroutine::coroutine_ptr s_coroutine;
             std::function<void()> s_function;
             event_context(){
                    s_scheduler=nullptr;
                    s_coroutine.reset();
                    s_function=nullptr;
             }
      };
       event();
       event(const int& fd);
       ~event();  
       event_context& GetContext(const event_type& type);//获取上下文类型
       void ResetContext(event_context& context);
       void TriggerEvent(event_type type);//将事件交给线程池执行
private:
       pthread_rwlock_t m_lock;//文件读写锁
       int m_fd;  
       event_context m_read;
       event_context m_write;
       event_type m_current;
};

#endif