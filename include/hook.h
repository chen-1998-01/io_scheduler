#ifndef _HOOK_H_
#define _HOOK_H_
#include<unistd.h>

//抓取系统调用的动态库文件或者可执行文件

namespace hook{
   void InitHook(); 
   void SetHookState(bool flage);
   bool IsEnableHook();
}
   
extern "C"{
   typedef unsigned int (*sleep_symbol)(unsigned int second);
   typedef int (*usleep_symbol)(useconds_t usecond);
   typedef size_t (*read_symbol)(int fd,void* buffer,size_t count);
   typedef size_t (*write_symbol)(int fd,const void* buffer,size_t count);
   extern sleep_symbol sleep_func;
   extern usleep_symbol usleep_func;
   extern read_symbol read_func;
   extern write_symbol write_func;    
}


#endif