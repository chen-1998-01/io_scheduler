#include"../../include/io.h"
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<cstring>
#include"../../include/logger.h"
//#include"../../include/memory_pool.h"
#include"../../include/hook.h"

static thread_local logger::_logger g_logger=root_logger;
//static thread_local memory_manager* g_memory=memory_manager::GetMemoryManger();

io_schedule::io_schedule(size_t thread_num,const std::string& name,bool Use_InitThread,int epoll_num)
:schedule(thread_num,name,Use_InitThread){
    m_handle=handle::GetHandle();
    m_timers.reset(new Timer_manager());        
    m_waitingevents={0};
    m_epoll_fd=epoll_create(epoll_num);
    epoll_event event;
    memset(&event,0,sizeof(event));
    event.data.fd=m_pipefds[0];
    event.events = EPOLLIN | EPOLLET;//可以读取，并且采用边缘触发模式
    int Pipe=pipe(m_pipefds);//创建管道
    fcntl(Pipe,F_SETFD,O_NONBLOCK);//设置为非阻塞态
    epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,m_pipefds[0],&event);
    hook::SetHookState(true);     
}

io_schedule::~io_schedule(){
   close(m_pipefds[0]);
   close(m_pipefds[1]);
   close(m_epoll_fd);
   for(int i=0;i<m_fds.size();i++){
      delete(m_fds[i]);
      m_fds[i]=nullptr;
   }
   m_fds.clear();
	logger_inform_eventstream(g_logger)<<"io_scheduler exit normally";
	ConsoleLog();
}

void io_schedule::resize(const int& size){
     if(size<m_fds.size())
       return;
    m_fds.reserve(size);
    for(int i=m_fds.size();i<size;i++){
        event* fd=new event();
        fd->m_fd=i;
        m_fds.push_back(fd);
    }
}

void io_schedule::AddTimer(std::function<void()> _func,const uint64_t& _time,bool _cyclic){    
      std::unique_lock<std::mutex>lock(m_mutex); 
      if(!m_timers.get())
        return;
      m_timers->AddTimer(_func,_time,_cyclic);  
}

void io_schedule::DelteTimer(std::shared_ptr<timer> _timer){    
      std::unique_lock<std::mutex>lock(m_mutex); 
      if(!m_timers.get())
        return;
      m_timers->DeleteTimer(_timer);  
}

void io_schedule::RegisterEvent(int fd,event_type type,std::function<void()>function){
    m_mutex.lock();
    event* temp;
    if(m_fds.size()>fd){
       temp=m_fds[fd];
    }
    else{ 
       resize(fd*2);       
       temp=m_fds[fd];
    }
    m_mutex.unlock();
    pthread_rwlock_wrlock(&temp->m_lock);
    event::event_context& _event=temp->GetContext(type);
    int op=(temp->m_current)?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
    epoll_event epollevent;
    memset(&epollevent,0,sizeof(epollevent));
    epollevent.events = (type | temp->m_current | EPOLLET);
    temp->m_current =(event_type) (type | temp->m_current);
    epollevent.data.ptr=temp;
    struct stat state;
    int infom=fstat(fd,&state);
    if(S_ISSOCK(state.st_mode)){
       logger_inform_eventstream(g_logger)<<"#socket: "<<fd<<" is normal";
       ConsoleLog(); 
    }       
    int value=epoll_ctl(m_epoll_fd,op,fd,&epollevent);//注册
    if(value==-1){
      logger_error_eventstream(g_logger)<<"#fd:"<<fd<<",#error:"<<strerror(errno);
      ConsoleLog();
      pthread_rwlock_unlock(&temp->m_lock);
      return; 
    }
    ++m_waitingevents; 
    _event.s_scheduler= schedule::get_this_schedule();
    if(function)
      _event.s_function.swap(function);
    else  
      _event.s_coroutine=coroutine::get_this_coroutine();
    pthread_rwlock_unlock(&temp->m_lock);         
    return;  
}

bool io_schedule::CancelEvent(int fd){
    m_mutex.lock();
    if(fd>=m_fds.size()){
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();    
    event* context=m_fds[fd];
   {
    pthread_rwlock_rdlock(&context->m_lock);  
    if(!context->m_current){
       pthread_rwlock_unlock(&context->m_lock);
       return false;
    }
    pthread_rwlock_unlock(&context->m_lock);
   }
    pthread_rwlock_wrlock(&context->m_lock);    
    int op=EPOLL_CTL_DEL;
    epoll_event epollevent;
    epollevent.events=0;
    epollevent.data.ptr=context;
    epoll_ctl(m_epoll_fd,op,fd,&epollevent);//取消事件   
    pthread_rwlock_unlock(&context->m_lock);
    return true;
}

bool io_schedule::DeleteEvent(int fd,event_type type){
   std::unique_lock<std::mutex> lock(m_mutex);
    if(fd<=m_fds.size())
       return false;
    event* _context=m_fds[fd];
    event_type _type=(event_type)(type & (_context->m_current));
    if(_type==event_type::NONE)
      return false;
    event::event_context& _event=_context->GetContext(_type);
    _context->ResetContext(_event);
    --m_waitingevents;     
    return true;   
}

bool io_schedule::can_stop(){
      if(!m_waitingevents && 
         m_activethreadnum && 
         m_coroutines.empty() &&
         m_timers->GetSize() && 
         m_autostop)
         return true;   
      return false;   
}

void io_schedule::notify(){
   if(!has_freethread()){
      return;
   }    
   char flage[16]="notify";
   std::unique_lock<std::mutex> lock(m_mutex);
   int value=write(m_pipefds[1],flage,strlen(flage));
   if(!value){
      logger_error_eventstream(g_logger)<<"notify failed";
	   ConsoleLog();
   }   
}//通过管道进行触发，唤醒其中一条已经被阻塞进入睡眠的线程。


void io_schedule::idle(){   
 if(!m_timers.get())
    return;   
 static const uint64_t MAX_TIMEOUUT=3000;    
 uint64_t next_time;
 struct epoll_event* events=new epoll_event[1024];
 static const int MAX_EVENTS=1024;
 int number;
 while(1){
      next_time=0ull;
      if(can_stop()){
         coroutine::coroutine_ptr current=coroutine::get_this_coroutine();
         coroutine* obj=current.get();
         current.reset();
         obj->set_state(coroutine::state::terminate); 
         break;
        }     
        next_time=m_timers->GetNextTime();
        if(next_time!=~0ull){
           next_time = next_time < MAX_TIMEOUUT ? next_time:MAX_TIMEOUUT;             
         } 
        else
           next_time=MAX_TIMEOUUT; 
        number=epoll_wait(m_epoll_fd,events,MAX_EVENTS,next_time); //阻塞线程，监听注册的fd       
        std::vector<std::function<void()>>functions;
        m_timers->GetExpiredList(functions);
        if(!functions.empty()){        
           batch_dispatch(functions.begin(),functions.end());
        }
      if(number){
       for(int i=0;i<number;i++){
          if(events[i].data.fd==m_pipefds[0]){
             char buffer[16];
             while(read(m_pipefds[0],buffer,6)==6);
             continue;
           }//管道消息触发
          struct epoll_event& _event=events[i];
          event* context=(event*)_event.data.ptr;
          if(!context)
             continue;
          pthread_rwlock_wrlock(&context->m_lock);//修改fd
          if((_event.events&context->m_current)==event_type::NONE){            
            pthread_rwlock_unlock(&context->m_lock);
            continue;
          }  
          int triggertype=0;         
          if(_event.events & event_type::READ)
             triggertype |= event_type::READ;
          if(_event.events & event_type::WRITE)
             triggertype |= event_type::WRITE;
          struct epoll_event leftevent;   
          event_type lefttype;
          lefttype=(event_type)(context->m_current & ~triggertype);
          int op = (lefttype!=event_type::NONE)?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
          epoll_ctl(m_epoll_fd,op,context->m_fd,&leftevent);//修改epoll_wait监听的事件    
          if(triggertype & event_type::READ){
             context->TriggerEvent(event_type::READ);
               --m_waitingevents;
          }
          if(triggertype & event_type::WRITE){
             context->TriggerEvent(event_type::WRITE);
               --m_waitingevents;
          }
           pthread_rwlock_unlock(&context->m_lock); 
           context=nullptr;    
      }
   }  
      functions.clear();
      if(events){
        delete[](events);
        events=nullptr;
      }
      coroutine::coroutine_ptr current=coroutine::get_this_coroutine();
      coroutine* obj=current.get();
      current.reset();
      if(m_timers->GetSize() || m_running)
        obj->set_state(coroutine::state::init);
      else
        obj->set_state(coroutine::state::terminate);    
      obj->swap_out();//切到调度器主协程
      continue; 
   }               
}

io_schedule* io_schedule::get_this_schedule(){
        return (io_schedule*)schedule::get_this_schedule();
}

fd_context::fdcontext_ptr io_schedule::GetFd(int fd){
   if(!m_handle)
     return nullptr;
   return m_handle->GetFd(fd);   
}