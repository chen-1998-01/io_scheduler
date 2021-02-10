#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include"lock.h"
#include<list>
#include<vector>

template<typename task>
class task_queue{
  protected:
     int m_maxnum;
     Mutex* m_mutex;
     condition_variable* m_condition;
     std::list<task>m_tasks;
     bool m_needstop;
  public:
      task_queue(int num);
      ~task_queue();
      void put_into(const task& _task);
      void put_into(std::list<task>&_tasks);
      void get_out(task& _task);
      void get_out(std::list<task>&_tasks);
      void carry_out();
      int get_size()const{
          return m_tasks.size();
      }
      void stop();
  protected:
       bool isempty()const{
            if(m_tasks.empty())
              return true;
            return false;  
       }
       bool isfull()const{
           return m_tasks.size()==m_maxnum?true:false;
       }
  protected:
        template<typename Task>
        void Put_into(Task&& _task){
          m_mutex->lock();
          std::string conduct_condition="not full";
          std::string consume_condition="not empty";
          while(isfull() || m_needstop)
          m_condition->wait_consume(conduct_condition);  
          m_tasks.push_back(std::forward<Task>(_task));
          //std::cout<<"the task is putted into"<<std::endl;
          m_condition->notify_one(g_consumer,consume_condition);
          m_mutex->unlock();
        }          
};

template<typename task>
task_queue<task>::task_queue(int num):
m_maxnum(num),m_needstop(false){
     m_mutex=new Mutex();
     m_condition=new condition_variable(*m_mutex);
     std::string consume_condition="not empty";
     m_condition->add_consumer(consume_condition);
     std::string conduct_condition="not full";
     m_condition->add_conductor(conduct_condition);
}

template<typename task>
void task_queue<task>::put_into(std::list<task>& _tasks){
    if(_tasks.size()>m_maxnum)
       return;
    m_mutex->lock();
    std::string conduct_condition="not full";
    std::string consume_condition="not empty";
    while(isfull() || m_needstop)
      m_condition->wait_consume(conduct_condition);  
    for(int i=0;i<_tasks.size();i++){
        task _task=_tasks.front();
        m_tasks.push_back(_tasks);
        _tasks.pop_front();
    }
    m_mutex->unlock();
    m_condition->notify_one(g_consumer,consume_condition);    
}

template<typename task>
void task_queue<task>::put_into(const task& _task){
   Put_into(std::move(_task));
}

template<typename task>
void task_queue<task>::get_out(task& _task){
    m_mutex->lock();
    std::string conduct_condition="not full";
    std::string consume_condition="not empty";
    while (isempty() || m_needstop)
       m_condition->wait_conduct(consume_condition);  
    _task=m_tasks.front();
    m_tasks.pop_front();
    //std::cout<<"the task is be getted out"<<std::endl;
    m_mutex->unlock();   
    m_condition->notify_one(g_conductor,conduct_condition);           
}

template<typename task>
void task_queue<task>::get_out(std::list<task>& _tasks){
    m_mutex->lock();
    std::string conduct_condition="not full";
    std::string consume_condition="not empty";
    while (isempty() || m_needstop)
       m_condition->wait_conduct(consume_condition);
    _tasks=std::move(m_tasks);
    m_mutex->unlock();
    m_condition->notify_one(g_conductor,conduct_condition);     
}

template<typename task>
void task_queue<task>::carry_out(){
  m_mutex->lock();
  std::string conduct_condition="not full";
  std::string consume_condition="not empty";
  while (isempty() || m_needstop)
    m_condition->wait_conduct(consume_condition);  
    task object=m_tasks.front();
    object();
    m_tasks.pop_front();
    m_mutex->unlock();   
    m_condition->notify_one(g_conductor,conduct_condition); 
}

template<typename task>
void task_queue<task>::stop(){
    m_mutex->lock();
    m_needstop=true;
    m_mutex->unlock();
    std::string conduct_condition="not full";
    std::string consume_condition="not empty";
    m_condition->notify_all(g_consumer,consume_condition);
    m_condition->notify_all(g_conductor,conduct_condition);
}

template<typename task>
task_queue<task>::~task_queue(){
    stop();
    delete(m_mutex);
    delete(m_condition);
    m_mutex=nullptr;
    m_condition=nullptr;
}

#endif