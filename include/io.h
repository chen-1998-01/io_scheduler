#ifndef _IO_H_
#define _IO_H_


//*该io调度器基于epoll多路复用进行设计
//*设置有事件容器，通过管道进行




#include"schedule.h"
#include"timer.h"
#include"event.h"
#include"handle.h"
#include<atomic>
#include<vector>

enum io_mode{
    Block_mode=0,
    NoBlock_mode=1
};//io采用的模式

class io_schedule:public schedule{
private:
   Timer_manager::timers_ptr m_timers;//定时器管理
   handle* m_handle;
   std::vector<event*>m_fds;//事件容器
   int m_epoll_fd;//epoll的fd号
   int m_pipefds[2];//管道的fd号，m_pipefds[0]负责监听，m_pipefds[1]负责触发
   std::atomic<uint32_t> m_waitingevents;//正在等待执行的事件数
   void resize(const int& size);
public:
    typedef std::shared_ptr<io_schedule> IoScheduler_ptr;
    io_schedule(size_t thread_num,const std::string& name,bool Use_InitThread=false,int epoll_num=1000);
    ~io_schedule();
    void notify() override;
    void idle() override;//从定时器中取出任务交给线程池来执行
    bool can_stop() override;
public:
    fd_context::fdcontext_ptr GetFd(int fd); 
    void RegisterEvent(int fd,event_type type,std::function<void()>); //事件注册
    bool CancelEvent(int fd);//取消某个fd下监听的所有事件
    bool DeleteEvent(int fd,event_type type);//删除事件容器里面的某个事件
    void AddTimer(std::function<void()>_func,const uint64_t& _time,bool _cyclic=false);
    void DelteTimer(std::shared_ptr<timer> _timer);
    static io_schedule* get_this_schedule();    
};//io调度器的设计



#endif