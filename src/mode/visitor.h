#ifndef _VISITOR_H_
#define _VISITOR_H_

#include<iostream>
#include<functional>
#include<vector>

namespace mode{
  
template<typename... types>
struct visitor;

template<typename element,typename... types>
struct visitor<element,types...>:public visitor<types...>{
  using visitor<types...>::visit;//避免基类同名方法被隐藏
  virtual void visit(const element& object)=0;
};
  
template<typename element> 
struct visitor<element>{
  virtual void visit(const element& _object)=0;
}; 
//visitor 接口类的定义(使用visitor作为接口)

struct concrete_element;//结构元素类的具体子类 

struct element_base{
    virtual ~element_base(){};
    typedef visitor<concrete_element> visitor_type;
    virtual void accept(visitor_type* _visitor)=0;
};
//被访问结构元素基类的定义

struct concrete_element:public element_base {
    void accept(visitor_type* _visitor)override{
       _visitor->visit(*this);
    }
};

struct concrete_visitor:public element_base::visitor_type{
      void visit(const concrete_element& _object)override{
         std::cout<<"visit "<<this<<std::endl;
      }
 };

}
#endif