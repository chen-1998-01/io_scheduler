#include"../../include/timer.h"
#include"../../include/times.h"


timer::timer(Timer_manager* manager,std::function<void()>func,const uint64_t& time,bool cyclic)
:m_manager(manager),m_func(func),m_delay(time),m_cyclic(cyclic){
     m_current=Time::GetCurrentMs();
     m_start=Time::GetCurrentMs()+m_delay;
}

timer::timer(const uint64_t& time)
:m_start(time),m_delay(0),m_current(time){          
}

timer::~timer(){
     m_func=nullptr;
}

void timer::refresh(){
     if(!m_manager)
        return;
     m_manager->DeleteTimer(shared_from_this());
     m_current=Time::GetCurrentMs();
     m_start=m_current+m_delay;
     m_manager->AddTimer(shared_from_this());      
}

void timer::reset(const uint64_t& time,bool from_now,std::function<void()>func){
     m_manager->DeleteTimer(shared_from_this());
     if(func)
       m_func=func;
     if(from_now)
       m_current=Time::GetCurrentMs();
     m_delay=time;
     m_start=m_current+m_delay;
     m_manager->AddTimer(shared_from_this());       
}


Timer_manager::Timer_manager()
:m_notify(false){
    m_previous=Time::GetCurrentMs();
}

Timer_manager::~Timer_manager(){
     pthread_rwlock_wrlock(&m_lock);
     m_timers.clear();
     pthread_rwlock_unlock(&m_lock); 
}

void Timer_manager::AddTimer(std::shared_ptr<timer> obj){
     pthread_rwlock_wrlock(&m_lock);
     m_timers.insert(obj);
     pthread_rwlock_unlock(&m_lock); 
}

std::shared_ptr<timer> Timer_manager::AddTimer(std::function<void()>func,const uint64_t& time,bool cyclic){
      pthread_rwlock_wrlock(&m_lock);
      std::shared_ptr<timer> obj(new timer(this,func,time,cyclic));
      m_timers.insert(obj);
      pthread_rwlock_unlock(&m_lock); 
      return obj;
}

bool Timer_manager::DeleteTimer(std::shared_ptr<timer> obj){
     std::set<std::shared_ptr<timer>,timer::comparator>::iterator target;
     {
      pthread_rwlock_wrlock(&m_lock);        
      target=m_timers.find(obj);
      if(target==m_timers.end()){
        pthread_rwlock_unlock(&m_lock);   
        return false;
      }
       m_timers.erase(target); 
       pthread_rwlock_unlock(&m_lock);
     }  
     return true;  
}


void Timer_manager::GetExpiredList(std::vector<std::function<void()>>& tasks){
    {
         pthread_rwlock_rdlock(&m_lock);
         if( m_timers.empty()){
              pthread_rwlock_unlock(&m_lock);
              return;
         }
         pthread_rwlock_unlock(&m_lock);
    }
         pthread_rwlock_wrlock(&m_lock);
         uint64_t now_time=Time::GetCurrentMs();
         if(!DetectClockAdjust(now_time) && *m_timers.begin() && now_time<(*m_timers.begin())->m_start){
              pthread_rwlock_unlock(&m_lock);
              return;
         }//没有超时任务
         std::vector<std::shared_ptr<timer>> expired;
         expired.clear();
         std::shared_ptr<timer> now_timer(new timer(now_time));
         auto i=DetectClockAdjust(now_time)?m_timers.end():m_timers.lower_bound(now_timer);
         while(i!=m_timers.end() && (*i)->m_start==now_time){
              i++;
         }     
         expired.insert(expired.begin(),m_timers.begin(),i);
         m_timers.erase(m_timers.begin(),i);
         tasks.reserve(expired.size());         
         for(auto& j:expired){
              tasks.push_back(j->m_func);
              if(j->m_cyclic){
                   j->m_current=now_time;
                   j->m_start=j->m_current+j->m_delay;
                   m_timers.insert(j);
              }
              else
                  j->m_func=nullptr;    
         }         
         pthread_rwlock_unlock(&m_lock);            
}

uint64_t Timer_manager::GetNextTime()const{
     if(m_timers.empty())
       return ~0ull;//没有任务需要执行
     uint64_t current=Time::GetCurrentMs();
     auto next=m_timers.begin();
     if(current>(*next)->m_start)
         return 0;
     return ((*next)->m_start)-current;
}

bool Timer_manager::DetectClockAdjust(const uint64_t& time){
    bool result=false;
    if(time<m_previous && time<m_previous-1000*60*60)
       result=true;
    m_previous=time;
    return result;   
}

void Timer_manager::AddUrgentEvent(std::shared_ptr<timer> event){
                   
}



