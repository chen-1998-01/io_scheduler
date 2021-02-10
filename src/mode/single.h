#ifndef _SINGLE_H_
#define _SINGLE_H_

#include<mutex>

namespace mode{

template<typename classname,typename... Args>
class nonauto_singleclass{
  private:
      static classname* m_instance;
  public:
      nonauto_singleclass(){};
      virtual ~nonauto_singleclass(){};
      static classname* get_instance(Args&&... args);
      void delete_instance();              
};

template<typename classname,typename... Args>
classname* nonauto_singleclass<classname,Args...>::get_instance(Args&&... args){
  std::mutex l_mutex;
  l_mutex.lock();
  if(!m_instance)
     m_instance=new classname(std::forward<Args>(args)...);
   l_mutex.unlock();  
  return m_instance;   
}

template<typename classname,typename... Args>
void nonauto_singleclass<classname,Args ...>::delete_instance(){
  std::mutex l_mutex;
  l_mutex.lock();
  if(m_instance){
    m_instance->~classname();
  }
  l_mutex.unlock();
}

template<typename classname,typename... Args>
classname* nonauto_singleclass<classname,Args ...>::m_instance=nullptr;




template<typename classname>
class  auto_singleclass{
  protected:
     static classname m_instance;
     static bool exist;
  public:
      static classname* get_instance();
      void delete_instance();   
};

template<typename classname>
classname* auto_singleclass<classname>::get_instance(){
    return &m_instance;
}

template<typename classname>
void auto_singleclass<classname>::delete_instance(){
   if(!exist)
      return;
   delete(get_instance());   
}
  
  template<typename classname>
  classname auto_singleclass<classname>::m_instance=classname();

}
#endif
