#include"../include/coroutine.h"
#include"../include/schedule.h"

static int value=0;
static void func(){
    value++;
}


int main(int argc,char* argv[]){
    std::function<void()>func_test=func;
    schedule::schedule_ptr scheduler_test(new schedule(3,"_test",true));   
    std::vector<std::function<void()>>functions_test;
    for(int i=0;i<9;i++)
       functions_test.push_back(func_test);
    scheduler_test->batch_dispatch(functions_test.begin(),functions_test.end());
     scheduler_test->start();
    scheduler_test->stop();   
    std::cout<<"task finished,the value:"<<value<<std::endl;       
    return 0;
}