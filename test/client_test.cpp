#include"../include/socket.h"
#include"../include/io.h"
#include"../include/logger.h"
#include<string.h>
#include<functional>

static thread_local logger::_logger g_logger = root_logger;

static int value=0;

void call_back(){
    logger_debug_eventstream(g_logger)<<"data write back";
    ConsoleLog();
}


int main(int argc, char* argv[]){
    io_schedule::IoScheduler_ptr io(new io_schedule(2,"io"));
    Socket::socket_ptr client_socket(new Socket(Socket::type::tcp,Socket::family::ipv4));
    std::string host="192.168.211.128";
    std::function<void()>func=&call_back;
    ipv4_address::address_ptr address;
    address=ipv4_address::create(host.c_str(),8096);
    io_schedule::get_this_schedule()->RegisterEvent(client_socket->GetFd(),event_type::WRITE,func);   
     io_schedule::get_this_schedule()->CancelEvent(client_socket->GetFd());
    io->start();
    do{      
      bool value=client_socket->Connect(address);
      if(value)
         break;
    }while(1); 
    io->stop();
    return 0;
}