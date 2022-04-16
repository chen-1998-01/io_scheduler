#include"../../include/hook.h"
#include<dlfcn.h>
#include"../../include/io.h"

static thread_local bool hook_state=false;


namespace hook{

#define HOOK(symbol)\
        symbol(sleep)\
        symbol(usleep)\
        symbol(read)\
        symbol(write)


void InitHook(){
  #define symbol(name) name ## _symbol name ## _func = (name ## _symbol)dlsym(RTLD_NEXT,#name);
  HOOK(symbol);
  #undef name 
}


void SetHookState(bool flage){
    hook_state=flage;
}

bool IsEnableHook(){
    if(hook_state)
      return true;
    return false;
}
}

extern "C"{
    #define symbol(name) name ## _symbol name ## _func = (name ## _symbol)dlsym(RTLD_NEXT,#name);
    HOOK(symbol);
    #undef name

    unsigned int sleep(unsigned int seconds){
        if(!hook::IsEnableHook())
           return sleep_func(seconds);   
        io_schedule* IO=io_schedule::get_this_schedule();
        if(!IO)
          return sleep_func(seconds);
        IO->AddTimer(std::bind(&schedule::run,IO),seconds*1000,false);
        coroutine* fiber=coroutine::get_this_coroutine().get();
        fiber->yieldtopause();   
        return 0;   
    }

    int usleep(useconds_t useconds){
        if(!hook::IsEnableHook())
           return usleep(useconds);  
         io_schedule* IO=io_schedule::get_this_schedule();
        if(!IO)
           return usleep(useconds); 
        IO->AddTimer(std::bind(&schedule::run,IO),useconds/1000,false);
        coroutine* fiber=coroutine::get_this_coroutine().get();
        fiber->yieldtopause();     
        return 0;  
    }

    ssize_t read(int fd, void* buffer, size_t count){
        if(!hook::IsEnableHook)
           return read_func(fd,buffer,count);
         io_schedule* IO=io_schedule::get_this_schedule();
         if(!IO)
           return read_func(fd,buffer,count);
         fd_context::fdcontext_ptr fdcontext=IO->GetFd(fd);
         if(!fdcontext)
            return read_func(fd,buffer,count);
         int size;   
         IO->RegisterEvent(fd,event_type::READ,
                           [&](){size=read_func(fd,buffer,count);});   
         IO=nullptr;
         return size;        
    }

    ssize_t write(int fd, const void* buffer, size_t count){
         if(!hook::IsEnableHook)
           return write_func(fd,buffer,count);
         io_schedule* IO=io_schedule::get_this_schedule();
         if(!IO)
           return write_func(fd,buffer,count);
         fd_context::fdcontext_ptr fdcontext=IO->GetFd(fd);
         if(!fdcontext)
            return write_func(fd,buffer,count);
         int size;   
         IO->RegisterEvent(fd,event_type::WRITE,
                           [&](){size=write_func(fd,buffer,count);});   
         IO=nullptr;
         return size;       
    }
}
