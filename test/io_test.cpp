#include"../include/init.h"
#include"../include/logger.h"
#include"../include/io.h"

static thread_local logger::_logger g_logger = root_logger;
static thread_local memory_manager* g_memory=memory_manager::GetMemoryManger();

static int value=0;

extern void load();

static void operate1(){
      hook::SetHookState(true);
      value++;
      sleep(1);                
}

static void operate2(){
      hook::SetHookState(true);
      value++;            
}

int main(int argc,char* argv[]){
      load();
      logger_inform_eventstream(g_logger)<<"process running";
      ConsoleLog();
      Thread::set_name("Main");
      std::function<void()>func1=operate1;
      std::function<void()>func2=operate2;
      io_schedule::IoScheduler_ptr IoScheduler_test(new io_schedule(2,"IO"));
      IoScheduler_test->start();
      IoScheduler_test->dispatch(func1);
      IoScheduler_test->dispatch(func2);
      IoScheduler_test->stop();
}