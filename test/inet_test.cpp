#include"../include/socket.h"
#include<iostream>
#include<cstring>
#include<memory>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>

static void test(){
    std::string host="192.168.211.128";
    ipv4_address::address_ptr test_ipv4=ipv4_address::create(host.c_str(),8096);
    Socket* test_socket=new Socket(Socket::type::tcp,Socket::family::ipv4,0,io_mode::Block_mode);
    test_socket->Bind(test_ipv4);
    test_socket->Listen();
    while(1){
        Socket::socket_ptr test= test_socket->Accept();       
        if(test.get())
           test->close();
    }
}

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

int main(int argc,char* argv[]){
    std::function<void()>task=test;
    init_daemon(task);  
    return 0;   
}