#include"../../include/command.h"
#include"../../include/init.h"
#include<functional>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<assert.h>
#include<string.h>

inline static void init_daemon(std::function<void()>task){
    pid_t process;
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGHUP,SIG_IGN);
    process=fork();
    if(process)
       exit(0);
    else if(process==-1)
       exit(1);   
    else if(!process){
        setsid();//产生新的会话，并且与父进程会话脱离
        process = fork();
        if(process)
           exit(0);
        else if(process==-1)
           exit(1);
        for(int i=0;i<1024;i++)
           close(i);     
        chdir("/");//改变工作目录
        umask(0);
        signal(SIGCHLD,SIG_IGN);        
        task();  
    } 
}


command::command(int argc,char* argv[]):
m_number(argc){
    int value=-1;
    if(argc>2){
      m_cmd=argv[2];
       value=strcmp(argv[1],"start");
     }   
    load();    
    std::function<void()>task;
    if(!value){
      assert(argc>2);
      task=std::bind(&command::implement,this);
      init_daemon(task); 
    }
    else if(value && argc>1)
      std::cout<<"command error"<<std::endl;   
}

command::~command(){

}

void command::implement(){
  Socket* _socket=new Socket(Socket::type::tcp,Socket::family::ipv4,0,io_mode::Block_mode);
  _socket->Bind();
  _socket->Listen();
  io_schedule::IoScheduler_ptr IO(new io_schedule(std::stoi(m_cmd),"IO"));    
  IO->start();
  while(1){
    Socket::socket_ptr test= _socket->Accept();       
    if(test.get())
    test->close();
    } 
}