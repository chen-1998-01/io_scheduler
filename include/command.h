#ifndef _COMMAND_H_
#define _COMMAND_H_

#include<unistd.h>
#include<string>

class command{
private:
   int m_process; 
   int m_number;
   std::string m_cmd;
public:
    command(int argc,char* argv[]);   
    ~command();
    void implement();     
};


#endif