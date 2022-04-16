#include"../../include/address.h"
#include"../../include/logger.h"
#include<cassert>




ipv4_address::ipv4_address(const sockaddr_in& _addr){
    m_address.sin_family=_addr.sin_family;
    m_address.sin_addr.s_addr=(_addr.sin_addr.s_addr);
    m_address.sin_port=(_addr.sin_port);   
}

ipv4_address::ipv4_address(const std::string& _addr,const uint16_t& _port){
     uint32_t addr=inet_addr(_addr.c_str());
     m_address.sin_addr.s_addr=(addr);
     m_address.sin_port=htons(_port);
     m_address.sin_family=AF_INET;
}

ipv4_address::ipv4_address(const uint32_t& _addr,const uint16_t& _port){
     m_address.sin_family=AF_INET;
     m_address.sin_addr.s_addr=htonl(_addr);
     m_address.sin_port=htons(_port); 
}

ipv4_address::~ipv4_address(){

}

int ipv4_address::get_family()const{
    return AF_INET;
}

const sockaddr* ipv4_address::get_addr()const{
    return (sockaddr*)&m_address;
}

socklen_t& ipv4_address::get_length(){
    static socklen_t length=sizeof(m_address);
    return length;
}

uint16_t ipv4_address::get_port()const{
    return ntohs(m_address.sin_port);
}

void ipv4_address::set_port(const uint16_t&  _port){
    m_address.sin_port=htons(_port);
}

void ipv4_address::print(std::ostream& stream){
    stream<<"[";
    uint32_t address=ntohl(m_address.sin_addr.s_addr);
    uint32_t temp=0;
    for(int i=0;i<4;i++){
        temp = (address >> (8*(3-i))) & (0xff);
        stream<<std::to_string(temp);
        if(i!=3)
           stream<<"."; 
    }
    stream<<"]";
    stream<<":"<<ntohs(m_address.sin_port)<<std::endl;
}

std::shared_ptr<net_address> ipv4_address::get_subnetmask(const uint32_t& prefix_len){
     sockaddr_in subnet_mask;
     uint32_t _addr;
     _addr = ~(0xffffffff >> prefix_len);       
     subnet_mask.sin_family=AF_INET;
     subnet_mask.sin_addr.s_addr=htonl(_addr);
     std::shared_ptr<net_address> subnet(new ipv4_address(subnet_mask));
     return subnet; 
}

std::shared_ptr<net_address> ipv4_address::get_boardaddress(const uint32_t& prefix_len){
    sockaddr_in board_sock;
    uint32_t _board;
    uint32_t _addr;  
    _addr=ntohl(m_address.sin_addr.s_addr);
    _board=_addr |= (0xffffffff >> prefix_len); 
    board_sock.sin_family=AF_INET;
    board_sock.sin_addr.s_addr=htonl(_board); 
     std::shared_ptr<net_address> board(new ipv4_address(board_sock));
     return board;
}

std::shared_ptr<net_address> ipv4_address::create(const char* ip,const uint16_t& _port){
    sockaddr_in socket_addr;
    socket_addr.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&socket_addr.sin_addr);
    socket_addr.sin_port=htons(_port);
    ipv4_address::address_ptr new_addr(new ipv4_address(socket_addr));
    return new_addr;              
}


ipv6_address::ipv6_address(){
    memset(&m_address,0,sizeof(m_address));
    m_address.sin6_family=AF_INET6;
}

ipv6_address::ipv6_address(const sockaddr_in6& _address){
    memset(&m_address,0,sizeof(m_address));
    m_address.sin6_family=AF_INET6;
    m_address.sin6_port=_address.sin6_port;
    memcpy(&m_address.sin6_addr.__in6_u,&_address.sin6_addr.__in6_u,16);
}

ipv6_address::ipv6_address(uint8_t _addr[16],const uint16_t& _port){
    memset(&m_address,0,sizeof(m_address));
    m_address.sin6_family=AF_INET6;
    memcpy(&m_address.sin6_addr.__in6_u,_addr,16);
    m_address.sin6_port=htons(_port);
}

ipv6_address::~ipv6_address(){

}

int ipv6_address::get_family()const{
   return AF_INET6;
}
const sockaddr* ipv6_address::get_addr()const{
   return (sockaddr*)&m_address;
}

socklen_t& ipv6_address::get_length(){
    static socklen_t length=sizeof(m_address);
    return length;
}

uint16_t ipv6_address::get_port()const{
   return ntohs(m_address.sin6_port);
}

void ipv6_address::set_port(const uint16_t& _port){
   m_address.sin6_port=htons(_port); 
}

void ipv6_address::print(std::ostream& stream){
   stream<<"[";
   uint16_t* addr =(uint16_t*) m_address.sin6_addr.__in6_u.__u6_addr16;
   bool use_zeros=false;
   for(int i=0;i<8;i++){
       if(addr[i]==0 && !use_zeros)
         continue;
       if(i  && addr[i-1]==0 && !use_zeros){
         stream<<":";
         use_zeros=true;
       }
       if(i)
          stream<<":";
       stream<<std::hex<<(addr[i])<<std::dec;
   }
   stream<<"]"<<":"<<ntohs(m_address.sin6_port)<<std::endl;       
}

std::shared_ptr<net_address> ipv6_address::get_boardaddress(const uint32_t& prefix_len){
      return nullptr;
}
        
std::shared_ptr<net_address> ipv6_address::get_subnetmask(const uint32_t& prefix_len){
      return nullptr; 
}

std::shared_ptr<net_address>  ipv6_address::create(const char* ip,const uint16_t& _port){
      sockaddr_in6 addr;
      addr.sin6_family=AF_INET6;
      inet_pton(AF_INET6,ip,&addr.sin6_addr);
      addr.sin6_port=htons(_port);
      ipv6_address::address_ptr new_address(new ipv6_address(addr));
      return new_address;
} 




bool net_address::operator==(const net_address& other){
    sockaddr _address=*get_addr();
    if(_address.sa_family!=other.get_family())
      return false;
    if(_address.sa_data!=other.get_addr()->sa_data)
      return false;
    return true;    
}

std::shared_ptr<net_address> net_address::create(sockaddr* _addr,const socklen_t&){
      if(_addr==nullptr)
         return nullptr;
      std::shared_ptr<net_address> address;
      switch(_addr->sa_family){
          case AF_INET:{
              address.reset(new ipv4_address(*((sockaddr_in*)(_addr))));
              break;
          }
          case AF_UNIX:{
              address.reset(new ipv6_address(*((sockaddr_in6*)(_addr))));     
              break;
          }
      }
      return address;
}
     
bool net_address::lookup(std::vector<address_ptr>& addresshub,const std::string& host,int family,int type,int protocal){
    return true;         
}

