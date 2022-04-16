#ifndef _TIMER_H_
#define _TIMER_H_

//定时器设计

#include<memory>
#include<functional>
#include<pthread.h>
#include<cstdint>
#include<set>
#include<list>
#include<vector>

class Timer_manager;

class timer:std::enable_shared_from_this<timer>{
friend class Timer_manager;    
private:
   Timer_manager* m_manager;
   std::function<void()>m_func;
   uint64_t m_current;
   uint64_t m_delay;
   uint64_t m_start;
   bool m_cyclic;     
public:
   typedef std::shared_ptr<timer> timer_ptr;
   timer(const uint64_t& time);
   timer(Timer_manager* manager,std::function<void()>func,const uint64_t& time,bool cyclic=false);
   ~timer();
   struct comparator{
       bool operator()(const timer_ptr& front,const timer_ptr& back)const{
           if(!front && !back)
             return true;
           else if(!front)
             return false;
           else if(!back)
             return true;   
           else
             return front->m_start <= back->m_start;  
       }
   };
   void refresh();
   void reset(const uint64_t& time,bool from_now=true,std::function<void()>func=nullptr);
};

class Timer_manager{    
private:
   std::set<std::shared_ptr<timer>,timer::comparator>m_timers;//采用红黑树进行管理
   pthread_rwlock_t m_lock;
   uint64_t m_previous;
   bool m_notify;     
public:
   Timer_manager();
   ~Timer_manager();
   typedef std::shared_ptr<Timer_manager> timers_ptr;
   void AddTimer(std::shared_ptr<timer>);
   std::shared_ptr<timer> AddTimer(std::function<void()>func,const uint64_t& time,bool cyclic=false);
   bool DeleteTimer(std::shared_ptr<timer>);
   void GetExpiredList(std::vector<std::function<void()>>& tasks);//定时收集超时任务，然后交给调度器来处理。
   uint64_t GetNextTime()const;
   bool DetectClockAdjust(const uint64_t& time);//检测服务器时间是否调整（gettimeofday获取的系统时间可能被ntp服务器调整）
   int GetSize()const{
      return m_timers.size();
   }
   static void AddUrgentEvent(std::shared_ptr<timer>);//添加紧急事件
};



#endif