#include"../../include/handle.h"
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>

fd_context::fd_context(int& fd):
  m_init(false),m_closed(false),
  m_SendTimeout(0),m_ReceiveTimeout(0){
    m_fd=fd;
    m_type=fd_type::UNKNOWN;
    init();
}

fd_context::~fd_context(){
   m_type=fd_type::UNKNOWN;
   if(!m_closed)
      close();
   m_SendTimeout=0;
   m_ReceiveTimeout=0;   
}

void fd_context::init(){
    if(m_closed){
      m_init=false;
      return;
    }
    struct stat state;
    int value=fstat(m_fd,&state);
    if(value==-1){
       m_init=false;
       return;
    }
    m_init=true;
    if(S_ISREG(state.st_mode))
      m_type=fd_type::ORDINARY;
    else if(S_ISBLK(state.st_mode) || S_ISCHR(state.st_mode))
       m_type=fd_type::DEVICE;
    else if(S_ISSOCK(state.st_mode))
        m_type=fd_type::SOCKET;
    else if(S_ISLNK(state.st_mode))
        m_type=fd_type::LINK;
    else
        m_type=fd_type::UNKNOWN;    
}

void fd_context::close(){
  if(m_closed)
     return;
  int value=fcntl(m_fd,FD_CLOEXEC);
  if(value==-1)
     return;  
   m_closed=true;
}

 void fd_context::SetTimeOut(int flage,const uint64_t& time_out){
   if(flage==SO_RCVTIMEO)
      m_ReceiveTimeout=time_out;
   else if(flage==SO_SNDTIMEO)
      m_SendTimeout=time_out;   
 }

 uint64_t fd_context::GetTimeOut(int flage){
   if(flage==SO_RCVTIMEO)
      return m_ReceiveTimeout;
   else if(flage==SO_SNDTIMEO)
      return m_SendTimeout;
   return 0;      
 }




handle* handle::m_handle=nullptr;

handle* handle::GetHandle(){  
  if(!m_handle)
     m_handle=new handle();   
  return m_handle;      
}

handle::handle(){
   m_fds.resize(16); 
}

bool handle::AddFd(int fd,fd_type type){    
    std::unique_lock<std::mutex>lock(m_mutex);
    if(fd>=m_fds.size())
       m_fds.resize(fd*2);
    fd_context::fdcontext_ptr new_fd(new fd_context(fd));
    if(new_fd->GetType()!=type)
       return false;
    m_fds[fd]=new_fd;   
    return true;      
}
      
bool handle::CancelFd(int fd,fd_type type){
    std::unique_lock<std::mutex>lock(m_mutex);
    if(m_fds.size()<=type)
       return false;
    if(!m_fds[fd].get())
       return false;   
    if(m_fds[fd]->GetType()!=type)
       return false;
    m_fds[fd].reset();           
    return true;  
} 

fd_context::fdcontext_ptr handle::GetFd(int fd){
   for(auto i=m_fds.begin();i<m_fds.end();i++){
      if((*i)->GetFd()==fd){
         return (*i);
      }     
   }
   return nullptr;  
}