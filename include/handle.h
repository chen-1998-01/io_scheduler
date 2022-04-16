#ifndef _HANDLE_H_
#define _HANDLE_H_
#include"thread.h"
#include<vector>

//句柄类的设计


enum fd_type{
  ORDINARY=0,
  DEVICE=1,
  SOCKET=2,
  LINK=3,
  UNKNOWN 
};//文件描述符的类型


class fd_context{
public:
     fd_context(int& fd);
     ~fd_context();
     typedef std::shared_ptr<fd_context> fdcontext_ptr;
     void init();
     void close();
     int GetFd()const{
           return m_fd;
     }
     fd_type GetType()const{
           return m_type;
     }
     bool IsInit() const{
           return m_init;
     }
     bool IsClose() const{
           return m_closed;
     }
     void SetTimeOut(int flage,const uint64_t& time_out);
     uint64_t GetTimeOut(int flage);
     bool operator==(const fd_context& fd){
           return m_fd==fd.GetFd();
     }
private:
     int m_fd;
     bool m_init;
     bool m_closed;
     fd_type m_type;
     uint64_t m_SendTimeout;
     uint64_t m_ReceiveTimeout;    
};


class handle{
private:
      std::mutex m_mutex;
      handle();
      static handle* m_handle;
      std::vector<fd_context::fdcontext_ptr>m_fds;
public:
      ~handle();
      static handle* GetHandle(); 
      bool AddFd(int fd,fd_type type);
      bool CancelFd(int fd,fd_type type);
      fd_context::fdcontext_ptr GetFd(int fd);         
};




#endif