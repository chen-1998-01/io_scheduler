#include"thread_pool.h"

thread_pool::thread_pool():
m_threadsize(0),m_queuesize(32),m_running(false){
    m_queue=new task_queue<task>(m_queuesize);   
}

 thread_pool::thread_pool(int threadnum,int queuesize):
  m_threadsize(threadnum),m_queuesize(queuesize),m_running(false){
       m_queue=new task_queue<task>(m_queuesize);   
  }

  void thread_pool::Start(){
       std::cout<<"start ..."<<std::endl;
       m_running=true;//切换状态
       m_threads.resize(m_threadsize);
       for(int i=0;i<m_threadsize;i++){
          m_threads[i].reset(new Thread(std::bind(&thread_pool::Run,this),"thread "+std::to_string(i)));
       }
  }
  
   void thread_pool::start(thread_pool* arg,int thread_num){
        arg->set_threadnum(thread_num);
        arg->Start(); 
   }

   void thread_pool::start(const int& threadnum){
        this->set_threadnum(threadnum);
        Start();
   }

   void thread_pool::run(){
        Run();
   }

   void thread_pool::Run(){
       std::cout<<"in the running state"<<std::endl;  
       while(m_running){  
         std::list<task>task_list;
         m_queue->get_out(task_list);
         for(auto Task:task_list){
           if(!m_running){
             std::cout<<"stop..."<<std::endl;
             return;
            }      
          Task();
         }
       }    
   }
   
   void thread_pool::stop(){
     std::call_once(m_flage,&thread_pool::Stop,this);
   }

   void thread_pool::Stop(){
        m_queue->stop();
        std::cout<<"begin to stop"<<std::endl;
        m_running=false;
        for(int i=0;i<m_threads.size();i++)
           if(m_threads[i])
             m_threads[i]->join();
        m_threads.clear();     
   }
   //任务处理接口,交给内部线程

   thread_pool::~thread_pool(){
        stop();
        m_threadsize=0;
        m_queuesize=0;
        m_queue->~task_queue<task>();
        m_queue=nullptr;
        for(int i=0;i<m_threads.size();i++)
           if(m_threads[i])
             m_threads[i].reset();
        m_threads.clear();       
   }
   
   void thread_pool::run(thread_pool* arg){
     arg->Run();
   }

   void thread_pool::stop(thread_pool* arg){
     arg->stop();
   }

   void thread_pool::add_task(task&& _task){
     m_queue->put_into(std::forward<task>(_task));
   }

   void thread_pool::add_task(const task& _task){
     m_queue->put_into(_task);
   }
   //任务添加接口，交给外部线程