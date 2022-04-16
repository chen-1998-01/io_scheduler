#include"../../include/event.h"

event::event()
:m_fd(0),m_current(event_type::NONE){
    pthread_rwlock_init(&m_lock,0);
};

event::event(const int& fd)
:m_fd(fd),m_current(event_type::NONE){
    pthread_rwlock_init(&m_lock,0);   
};

event::~event(){
    pthread_rwlock_destroy(&m_lock);
    m_current=event_type::NONE;
}

event::event_context& event::GetContext(const event_type& type){
    static event_context Null_context;
    switch (type){
      case event_type::READ:{
          return m_read;
      }
      case event_type::WRITE:{
          return m_write;
      }
    }
    return Null_context;  
}

void event::ResetContext(event_context& context){
    context.s_coroutine=nullptr;
    context.s_function=nullptr;
    context.s_coroutine.reset();
}

void event::TriggerEvent(event_type type){
    m_current=(event_type)(m_current & ~type);//修改fd上下文   
    event_context& context=GetContext(type);
    if(context.s_function)
       context.s_scheduler->dispatch(&context.s_function);
    if(context.s_coroutine)
       context.s_scheduler->dispatch(&context.s_coroutine);
    context.s_scheduler=nullptr;            
}

