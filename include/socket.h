#ifndef _SOCKET_H_
#define _SOCKET_H_

#include"io.h"
#include"address.h"
#include"handle.h"


enum actor{
   server = 0,
   client = 1
};

class Socket{
    public:
     enum family{
       ipv4=AF_INET,
       ipv6=AF_INET6
     };
     enum type{
         tcp=SOCK_STREAM,
         udp=SOCK_DGRAM
     };      
  public:
     Socket(type, family, int protocol=0, io_mode mode=io_mode::NoBlock_mode);
     ~Socket();
     typedef std::shared_ptr<Socket> socket_ptr;
     bool Bind(const char*,uint16_t);
     bool Bind(net_address::address_ptr address=nullptr);//绑定本地套接字
     bool Connect(const net_address::address_ptr address,uint64_t time=-1);//主动请求连接
     void Listen(const uint32_t& number=1024);//监听
     std::shared_ptr<Socket> Accept();//接受
     void BreakConnect();
     bool close();//关闭
     void dump(std::ostream& stream);
     bool connecting()const{
        return m_isconnected;
     };
     int GetFd()const{
        return m_socket;
     }
  public:
     bool Send(const void* buffer,const size_t& size,net_address::address_ptr address=nullptr);
     bool Receive(void* buffer,size_t size,net_address::address_ptr address=nullptr);
     int32_t Write(void* buffer,size_t size,actor mode,const int& fd=0);
     int32_t Read(void* buffer,size_t size,actor mode,const int& fd=0);
  private:
     net_address::address_ptr m_local_address;
     net_address::address_ptr m_remote_address;
     bool m_isconnected;
     bool m_isbind;    
     int m_socket;
     family m_family;
     type m_type;
     int m_protocol;
     int m_mode;
  private:
     void InitSocket();
     bool InitSocket(const int& fd);//新创建一个socket
     net_address::address_ptr GetLocalAddress();
     net_address::address_ptr GetRemoteAddress();
};





#endif