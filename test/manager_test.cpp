#include"../src/manager/memory_pool.h"
#include"../src/manager/times.h"
#include"../src/manager/logger.h"

class t_class{
  private:
    int m_value;
  public:
    t_class():m_value(0){};
    t_class(const int& value):m_value(value){};
    ~t_class(){};
    void print()const{
      std::cout<<"the test value is#"<<m_value<<std::endl;
    }
};

void t_function(){
  int i=0;
  while(i<1024)
    i++;
}

int main(int argc,char* argv[]){
  class_memory_pool* t_pool=new class_memory_pool();
  t_class* t_instance=t_pool->default_new_class<t_class>();
  t_pool->delete_class<t_class>(t_instance);
  t_class* t_instance2=t_pool->default_new_class<t_class>();
  int value=1;
  t_class* t_instance3=t_pool->new_class<t_class>(value);
  t_instance3->print();
  std::cout<<"#memory pool test#"<<std::endl;
  Time* t_times=new Time();
  std::cout<<t_times->get_formate_date();
  std::function<void()>t_func=t_function;
  t_times->get_elapsetime(t_func);
  std::cout<<"#time test#"<<std::endl;
  logger::_logger t_logger=root_logger();
  logevent::_event t_event(new logevent(__FILE__,__LINE__,"unknow",0,0,0,0,logger_level::debug,t_logger));
  t_logger->log(logger_level::debug,t_event);
  std::cout<<"#logger test#"<<std::endl;
  return 0;
}