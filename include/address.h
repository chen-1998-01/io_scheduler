#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<memory>
#include<vector>
#include<string>
#include<map>

class net_address{
    public:
         virtual ~net_address(){};    
         using  address_ptr=std::shared_ptr<net_address>;
         static address_ptr create(sockaddr*,const socklen_t&);
         //创建地址
         static bool lookup(std::vector<address_ptr>& addrhub,const std::string& host,int family,int type=0,int protocal=0);
        //根据域名和网络协议族来查看网络地址
         virtual const sockaddr* get_addr()const=0;
         virtual void print(std::ostream& stream)=0;
         virtual int get_family()const=0;
         bool operator ==(const net_address&);
         virtual uint16_t get_port()const=0;
         virtual void set_port(const uint16_t&)=0;
         //设置端口
         virtual address_ptr get_boardaddress(const uint32_t& prefix_len)=0;
         //获取广播地址
         virtual address_ptr get_subnetmask(const uint32_t& prefix_len)=0;
         //获取子网掩码
         virtual socklen_t& get_length()=0;
    private:
        static std::map<std::string,address_ptr> m_directories;    
};

class ipv4_address:public net_address{
     private:
        sockaddr_in m_address;   
     public:
        ipv4_address(const sockaddr_in&);
        typedef std::shared_ptr<ipv4_address> ipv4_ptr;
        ipv4_address(const uint32_t& _addr=INADDR_ANY,const uint16_t& _port=8096);
        //根据32位二进制形式地址生成网络地址
        ipv4_address(const std::string& _addr,const uint16_t& _port=8096);
        //根据点分式形式地址生成网络地址
        ~ipv4_address();
        int get_family()const override;
        const sockaddr* get_addr()const override;
        socklen_t& get_length() override;
        uint16_t get_port()const override;
        void set_port(const uint16_t&) override;
        void print(std::ostream&) override;
        address_ptr get_boardaddress(const uint32_t& prefix_len) override;
        address_ptr get_subnetmask(const uint32_t& prefix_len) override;
        static  std::shared_ptr<net_address>  create(const char* ip,const uint16_t& _port=0);
};



class ipv6_address:public net_address{
   private:
       sockaddr_in6 m_address;
   public:
       ipv6_address();
       ipv6_address(const sockaddr_in6&);
       typedef std::shared_ptr<ipv6_address> ipv6_ptr;
        ipv6_address(uint8_t _addr[16],const uint16_t& _port=0);
        //根据点分式形式地址生成网络地址
        ~ipv6_address();
        int get_family()const override;
        const sockaddr* get_addr()const override;
        socklen_t& get_length() override;
        uint16_t get_port()const override;
        void set_port(const uint16_t&) override;
        void print(std::ostream&) override;
        address_ptr get_boardaddress(const uint32_t& prefix_len) override;
        address_ptr get_subnetmask(const uint32_t& prefix_len) override;
        static  std::shared_ptr<net_address>  create(const char*,const uint16_t& _port=0);                     
};

class local_address{

};

#endif