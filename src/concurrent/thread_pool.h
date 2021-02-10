#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include"task_queue.h"
#include"thread.h"  
#include<atomic>
#include<mutex>

class thread_pool{
   public:
      using task=std::function<void()>;
      thread_pool();
      thread_pool(int threadnum,int queuesize);
      ~thread_pool();
      static void start(thread_pool* arg,int thread_num);
      static void run(thread_pool* arg);
      static void stop(thread_pool* arg);
   public:
       void add_task(task&& _task);
       void add_task(const task& _task);
       void set_threadnum(const int& thread_num){
          m_threadsize=thread_num;
       }
       void run();
       void start(const int& threadnum);
       void stop();
   protected:
      void Start();   
      void Run();
      void Stop();
   private:
     int m_threadsize;
     int m_queuesize;
     std::atomic_bool m_running;
     //设置为原子变量，采取原子操作
     std::once_flag m_flage;
     std::vector<Thread::thread_ptr>m_threads;
     task_queue<task>* m_queue;  
};
  
 
#endif