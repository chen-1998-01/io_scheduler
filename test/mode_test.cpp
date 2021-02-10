#include"../src/mode/single.h"
#include"../src/mode/observer.h"
#include"../src/mode/visitor.h"
#include"../src/mode/command.h"

class t_class:public mode::nonauto_singleclass<t_class>{
  private:
      int m_value;
  public:
    t_class():m_value(0){
      std::cout<<"the test class is loading"<<std::endl;
    }
    void operation(){
      m_value++;
      std::cout<<m_value<<std::endl;
    }
};

class t_class2:public mode::auto_singleclass<t_class2>{
  private:
    int m_value;
  public:
    t_class2():m_value(0){
      std::cout<<"the test class2 is loading"<<std::endl;
    }
    void operation(){
         m_value++;
         std::cout<<m_value<<std::endl;
    }
};

 void t_print(int a){
      std::cout<<a<<std::endl;
 }

int main(int argc,char* argv[]){
      t_class* t_instance=t_class::get_instance();
      for(int i=0;i<2;i++)
         t_instance->operation();
      t_class2* t_instance2=t_class2::get_instance();
      std::cout<<"#single pattern test#"<<std::endl;
      //单例类测试
      mode::observed<std::function<void(int)>>* t_observepoint=new mode::observed<std::function<void(int)>>;
      std::function<void(int)>t_function=t_print;
      t_observepoint->assign(t_function);
      int value=1;
      t_observepoint->notify_all(value);
      std::cout<<"#observer pattern test#"<<std::endl;
      //观察者类测试
      mode::concrete_visitor* t_visitor=new mode::concrete_visitor();
      mode::concrete_element* t_element=new mode::concrete_element();
      t_element->accept(t_visitor);
      std::cout<<"#visitor pattern test#"<<std::endl;
      //参观者类测试
      return 0;
}