#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include<functional>
#include<vector>
#include<map>

namespace mode{

template<typename function>
class observed{
  protected:
     std::map<int,function>m_observerlist;
     int m_num;
  protected:
    template<typename special_function>
       bool Assign(special_function&& object){
          m_num++;
          m_observerlist.emplace(m_num,object);
          return true;
       }//注册

    template<typename special_function>
       bool Cancel(const uint& index){
           if(m_observerlist.empty() || index>=m_observerlist.size())
              return false;
           m_observerlist.erase(index);         
       }//撤销

  public:
       observed():m_num(0){};
       void assign(function&& );
       void assign(const function&);
       void cancel(const function&);

    template<typename... arg_list>
       void notify_all(arg_list&&... args){
          for(auto i:m_observerlist){
            i.second(std::forward<arg_list>(args)...);
          }
       }
       //通知所有观察者的委托函数对象进行实例化

    template<typename... arg_list>
       void notify_one(const uint& index,arg_list&&... args){
           typename std::map<int,function>::iterator i=m_observerlist.begin();
           uint j=0;
           while (i!=m_observerlist.end()){
               if(j==index){
                  i->second(std::forward<arg_list>(args)...);
                  break;}
               j++;  
           }           
       }   
};

template<typename function>
void observed<function>::assign(function&& object){
     Assign(object);
}

template<typename function>
void observed<function>::assign(const function& object){
     Assign(object); 
}

template<typename function>
void observed<function>::cancel(const function& object){
     Cancel(object);
}

}
#endif