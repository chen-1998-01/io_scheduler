#ifndef _FILE_STREAM_H_
#define _FILE_STREAM_H_

#include<iostream>
#include<cstdio>
#include<fstream>
#include<string>
#include<cstring>
#include<unistd.h>
#include<mutex>
#include<map>
#include"../mode/single.h"

template<typename T>
bool read_stream(std::istream& in,const T& data,const int size){
   uint position=0;
   while(in && position<size){
       in.read((char*)(data+position),size-position);
       position+=in.gcount();
   }
   return position==size;     
}

template<typename T>
bool write_stream(std::ostream& out,T& data,const int size){
    out.write((const char*)data,size);
    return (bool)out;
}


class file:public mode::nonauto_singleclass<file>{
  private:
    struct file_s{
      enum pattern{
        read_write=0,
        only_read=1,
        only_write=2
      };
      std::string name;
      int fd;
      std::fstream file_stream;
      std::string information;
      bool opened;
      file_s(){};
      file_s(const std::string& _name):
      name(_name),fd(0),opened(false){};
    };
    typedef struct file_s* file_p;
    int m_maxnum;
    int m_filenum;
    file_s** m_files;
    std::map<std::string,int>m_directionary;
    std::mutex m_mutex;
  private:
     bool open_file(const int& index,file_s::pattern _pattern=file_s::pattern::read_write);
     bool close_file(const int& index);
     bool exist(const std::string& _name);  
  public:
    file(int num=64);
    ~file();
    void add_file(const std::string& _name);
    bool open_file(const std::string& _name,file_s::pattern _pattern=file_s::pattern::read_write);
    bool close_file(const std::string& _name);
    void read_file(const std::string& _name,std::string& _data);
    void delete_file(); 
    std::fstream* get_stream(const std::string& _name);   
};


#endif