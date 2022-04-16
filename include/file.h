#ifndef _FILE_H_
#define _FILE_H_

#include<iostream>
#include<cstdio>
#include<fstream>
#include<string>
#include<cstring>
#include<unistd.h>
#include<mutex>
#include<map>

template<typename T>
bool read_stream(std::istream& in,const T& data,const int size){
   uint position=0;
   while(in && position<size){
       in.read((char*)(data+position),size-position);
       position+=in.gcount();
   }
   return position==size?true:false;     
}

template<typename T>
bool write_stream(std::ostream& out,T& data,const int size){
    out.write((const char*)data,size);
    return (bool)out;
}


class file_manager{
  private:
    struct file_s{
        enum access{
            r=0,
            w=1,
            e=2,
            rw=3,
            re=4,
            we=5,
            rwe=6
            };
        enum type{
             ordinary=0,
             device=1,
             socket=2,
             pipe=3,
             link=4  
        };   
        std::string name;
        std::fstream* file_stream;
        int fd;
        std::string information;
        bool opened;
        file_s(){};
        file_s(const std::string& _name):
        name(_name),fd(0),opened(false){};};
   //文件的结构

    typedef file_s* file_p;
    int m_maxnum;//最多文件管理数量
    int m_filenum;//当前文件数量
    file_s** m_files;//存储管理文件
    std::map<std::string,int>m_catalogue;//存储文件索引
    std::mutex m_mutex;

  private:
    file_manager(int num=64); 
     bool open_file(const uint& index,file_s::access _access=file_s::access::r);
     bool close_file(const uint& index);
     int exist(const std::string& _name);  
  
  public:
    static file_manager* FileManager; 
    static file_manager* GetFileManager();
    ~file_manager();
    enum io_operate{
        read=0,
        write=1
    };
    void add_file(std::string _name);
    bool open_file(const std::string& _name,file_s::access _access=file_s::access::r);
    bool close_file(const std::string& _name);
    bool rw_file(const std::string& _filename,std::string& _buffer,io_operate type);
    void delete_file(); 
    std::fstream* get_stream(const std::string& _name);   
};


#endif