#ifndef _INIT_H_
#define _INIT_H_

//**启动时进行初始化**

#include"times.h"
#include"memory_pool.h"
#include"file.h"
#include"io.h"
#include"socket.h"
#include"hook.h"



void load(){
   Time::GetTimeManager();
   memory_manager::GetMemoryManger();
   file_manager::GetFileManager();
   Thread::set_name("start server...");
   std::cout<<Thread::get_this_threadname()<<std::endl;
   hook::SetHookState(true);
}



#endif