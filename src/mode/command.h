#ifndef _COMMAND_H_
#define _COMMAND_H_

#include<functional>
#include<type_traits>
#include<vector>
#include<list>
#include<mutex>

namespace mode{

template<typename return_value>
struct request{
  protected:
     std::function<return_value()>m_functions;
  public:   
     template<class function,class... arg,class=typename std::enable_if<!std::is_member_function_pointer<function>::value>::type>
     void wrap( function && _function,arg && ... args){
       m_functions=_function(std::forward<arg>(args)...);
     }      
     //针对非成员函数的包装

     template<class classname,class function,class... function_arg,class instance,class... arglist>
     void wrap(return_value(classname::*functionname)(function_arg...),instance&& object,arglist&& ... args){
       m_functions=(*object.*functionname)(std::forward<arglist>(args)...);
     } 
     //针对类成员函数的包装
     //第一个参数中包含了返回类型，类名，函数名（因为支持重载，所以要表明参数类型），第二个参数表示类的实例，第三个参数表示传递实际参数值
     
     return_value call(){
       return m_functions;
     }
  };

struct communicator{
    protected:
       std::vector<request<void>*>m_requests;
       int m_num;
    public:
       communicator():m_num(0){};
       virtual ~communicator(){};
       virtual void add_request(request<void>*& _request)=0;  //添加请求
       virtual void add_request(std::function<void()>& _function)=0;
       virtual void deal_request(request<void>*& _request)=0;  //处理请求
       int get_requestnum()const;   
  };//传达者

struct executor{
    protected:
       std::vector<communicator*>m_communicators;  
    public:
       executor();
       virtual ~executor(){};
       virtual void execute(communicator*)=0;
  };//执行者



}

#endif