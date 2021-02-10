#include"../src/concurrent/lock.h"
#include"../src/concurrent/thread.h"
#include"../src/concurrent/task_queue.h"
#include"../src/concurrent/thread_pool.h"
//#include"../src/concurrent/coroutine.h"
//#include"../src/concurrent/schedule.h"
#include<thread>


Mutex* t_mutex=new Mutex();
condition_variable* t_condition=new condition_variable(*t_mutex);
int i=0;
int j=0;

void consumer(){
  t_mutex->trylock();
  std::string condition="i大于1";
  while(i<1)
    t_condition->wait_conduct(condition);//被阻塞，等待通知  
  std::cout<<i<<" ";
  t_mutex->unlock();
}

void conducter(){
  t_mutex->trylock();
  std::string condition="i大于1";
  i++;
  t_condition->notify_one(g_consumer,condition);
  t_mutex->unlock(); 
}

void task(){
   for(int i=0;i<3;i++){
     j++;
     std::cout<<j<<" ";
   }
   std::cout<<std::endl;
}

int main(int argc,char* argv[]){
  std::string condition="i大于1";
  t_condition->add_consumer(condition);
  std::vector<Thread::thread_ptr>t_threads;
  t_threads.resize(6);
  for(int j=0;j<3;j++){
    t_threads[2*j].reset(new Thread(consumer,"consumer thread "+std::to_string(j)));
    t_threads[2*j+1].reset(new Thread(conducter,"conductor thread "+std::to_string(j)));
  }
  for(int j=0;j<t_threads.size();j++)
     t_threads[j]->join();   
  std::cout<<"#lock test#"<<std::endl;
  std::cout<<std::this_thread::get_id()<<std::endl;
  task_queue<std::function<void()>>* t_queue=new task_queue<std::function<void()>>(9);
  std::vector<std::thread>t_threadlist;
  for(int i=0;i<3;i++){
     t_threadlist.push_back(std::thread(&task_queue<std::function<void()>>::carry_out,t_queue));
     using function_type=void(task_queue<std::function<void()>>::*)(const std::function<void()>&);
     function_type add=&task_queue<std::function<void()>>::put_into;
     t_threadlist.push_back(std::thread(add,t_queue,task));
  }
  for(int i=0;i<t_threadlist.size();i++)
    t_threadlist[i].join();  
  std::cout<<"#task queue test#"<<std::endl;
  j=0;
  t_threadlist.clear();
  thread_pool* t_pool=new thread_pool;
  std::cout<<"add task"<<std::endl;
  for(int k=0;k<3;k++){
     using function_type=void(thread_pool::*)(std::function<void()>&&);
     function_type add=&thread_pool::add_task;
     t_threadlist.push_back(std::thread(add,t_pool,task));
  }
  t_pool->start(3);  
  for(int k=0;k<t_threadlist.size();k++)
      t_threadlist[k].join();       
  std::cout<<"#thread pool test#"<<std::endl;
  return 0;
}