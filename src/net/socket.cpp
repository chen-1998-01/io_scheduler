#include"../../include/socket.h"
#include"../../include/io.h"
#include"../../include/logger.h"
#include"../../include/handle.h"
#include"../../include/hook.h"
#include<unistd.h>
#include<fcntl.h>


static thread_local logger::_logger g_logger = root_logger;


Socket::Socket(type _type,family _family,int _protocol,io_mode mode)
:m_isbind(false),m_isconnected(false),
 m_family(_family),m_type(_type),m_protocol(_protocol),
 m_mode(mode){
   m_socket=socket(_family,_type,_protocol);    
   if(m_mode==io_mode::NoBlock_mode){
      fcntl(m_socket,F_SETFL,O_NONBLOCK);
   }
   if(m_socket==-1 && m_mode==io_mode::Block_mode){
      logger_error_eventstream(g_logger)<<"socket created failed";
      ConsoleLog();
   }
   else if(m_socket!=-1){
    logger_debug_eventstream(g_logger)<<"#socket: "<<m_socket<< " created successfully";
    ConsoleLog();
   }  
}         
    
Socket:: ~Socket(){
   close();
}

net_address::address_ptr Socket::GetLocalAddress(){
     if(m_local_address)
        return m_local_address;
     switch(m_family){
        case family::ipv4:{
            m_local_address.reset(new ipv4_address());
            break;
        }
        case family::ipv6:{
            m_local_address.reset(new ipv6_address());
            break;
        }
     }
     return m_local_address; 
}

net_address::address_ptr Socket::GetRemoteAddress(){
      if(m_remote_address)
        return m_local_address;
      switch(m_family){
        case family::ipv4:{
            m_remote_address.reset(new ipv4_address());
            break;
        }
        case family::ipv6:{
            m_remote_address.reset(new ipv6_address());
            break;
        }
     }
      return m_remote_address;
}

void Socket::InitSocket(){
      m_local_address.reset();
      m_remote_address.reset();
}

bool Socket::InitSocket(const int& fd){
     handle* fds=handle::GetHandle();
     if(fds->AddFd(fd,fd_type::SOCKET)==false)
        return false;
     m_socket=fd;
     GetLocalAddress();
     GetRemoteAddress();
     m_isconnected=true;
     return true;
}

bool Socket::Bind(const char* ip,uint16_t port){
   switch(m_family){
       case family::ipv4:{
           m_local_address=ipv4_address::create(ip,port);
           break;;
       }
       case family::ipv6:{
           m_local_address=ipv6_address::create(ip,port);
           break;
       }
   }
     int value=bind(m_socket,m_local_address->get_addr(),(socklen_t)sizeof(*m_local_address->get_addr()));
     if(value==-1){
       logger_error_eventstream(g_logger)<<"address bind failed";
       ConsoleLog();
       return false;
     }  
     return true;       
}
     
bool Socket::Bind(net_address::address_ptr address){
   if(address==nullptr){
      GetLocalAddress();
   }
   else{ 
      if(m_local_address.get() || m_isconnected)
        return false;     
      if(address->get_family()!=m_family)
        return false;
      m_local_address.reset(address.get());
     }  
    int value=bind(m_socket,m_local_address->get_addr(),(socklen_t)sizeof(struct sockaddr));
    if(value==-1){
      logger_error_eventstream(g_logger)<<"address bind failed";
       ConsoleLog();
       return false;
    }   
      logger_debug_eventstream(g_logger)<<"address bind successfully";
      ConsoleLog();   
      return true;         
}
     
bool Socket::Connect(const net_address::address_ptr address,uint64_t time){
    if(address->get_family()!=m_family)
      return false;
    m_remote_address=address;
    int value; 
    value=connect(m_socket,m_remote_address->get_addr(),(socklen_t)sizeof(*m_remote_address->get_addr()));
    if(value==-1){
      logger_error_eventstream(g_logger)<<"address connected failed";
      ConsoleLog();
      return false;
    }
    logger_debug_eventstream(g_logger)<<"address connected successfully";
    ConsoleLog();
    m_isconnected=true;
    return true;  
}
     
void Socket::Listen(const uint32_t& number){
     listen(m_socket,number);     
}


std::shared_ptr<Socket> Socket::Accept(){
     int fd=accept(m_socket,nullptr,nullptr);
     if(fd==-1){
       if(m_mode==io_mode::Block_mode){
         logger_error_eventstream(g_logger)<<"address refused connected";
         ConsoleLog();
       }
       return nullptr;
     }
     //fd返回为一个client请求的fd，交给任务管理器来处理
     Socket::socket_ptr new_socket(new Socket(m_type,m_family));
    if(new_socket->InitSocket(fd)){
        logger_inform_eventstream(g_logger)<<"accept address request,#fd: "<<fd;
        ConsoleLog();
        m_isconnected=true;
        return new_socket;
    }    
    return nullptr;
}//accept后会产生一个新的socket fd
     
bool Socket::close(){
    if(!connecting())
      return false;
    ::close(m_socket);
    m_isconnected=false;
    logger_inform_eventstream(g_logger)<<"#socket: "<<m_socket<<" been closed";
    ConsoleLog();
    return true;
}

void Socket::dump(std::ostream& stream){
    stream<<strerror(errno)<<std::endl;
}

 bool Socket::Send(const void* buffer,const size_t& size,net_address::address_ptr address){
     if(!m_isconnected)
        return false;
     if(!address)        
       send(m_socket,buffer,size,0);
     else
       sendto(m_socket,buffer,size,0,address->get_addr(),sizeof(address->get_addr()));
     return true;
 }

bool Socket::Receive(void* buffer,size_t size,net_address::address_ptr address){
     if(!address)
        recv(m_socket,buffer,size,0);
     else{
         socklen_t* length=&(address->get_length());
         recvfrom(m_socket,buffer,size,0,(sockaddr*)address->get_addr(),length);
     }       
     return true;
}
     
int32_t Socket::Write(void* buffer,size_t size,actor mode,const int& fd){
    uint32_t value;
    if(mode==actor::server){
      if(!m_isconnected)
        return -1;
      value=write(fd,buffer,size);
    }
    else if(mode==actor::client){
      if(!m_isconnected)
        return -1;  
      value=write(m_socket,buffer,size);             
    }
    return value;
}
     
int32_t Socket::Read(void* buffer,size_t size,actor mode,const int& fd){
    uint32_t value;
    if(mode==actor::server){
      if(!m_isconnected)
        return -1;
      value=read(fd,buffer,size);
      if(value==-1){
         logger_error_eventstream(g_logger)<<"#fd:"<<fd<<" #error:"<<strerror(errno);
         ConsoleLog();
      }        
    }
    else if(mode==actor::client){
      if(!m_isconnected)
         return -1;
      value=read(m_socket,buffer,size);
      if(value==-1){
         logger_error_eventstream(g_logger)<<"#fd:"<<fd<<" #error:"<<strerror(errno);
         ConsoleLog();
      }                     
    }
    return value;
}

void Socket::BreakConnect(){
   return;
}


