#include"../include/timer.h"
#include<iostream>

int main(int argc,char* argv[]){
    Timer_manager::timers_ptr timers_test(new Timer_manager());
    std::cout<<timers_test->GetNextTime()<<std::endl;
    return 0;
}