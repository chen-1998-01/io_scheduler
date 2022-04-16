#include"../../include/file.h"

file_manager* file_manager::FileManager=nullptr;

file_manager* file_manager::GetFileManager(){
  if(!FileManager)
     FileManager=new file_manager();
   return FileManager;  
}

file_manager::file_manager(int num):m_maxnum(num),m_filenum(0){
   m_files=new file_p[m_maxnum];
   for(int i=0;i<m_maxnum;i++)
     m_files[i]=nullptr;
}

file_manager::~file_manager(){
   if(m_files){
     delete_file();
   }
}

int file_manager::exist(const std::string& _name){
     int result=-1;
     for(auto i=m_catalogue.begin();i!=m_catalogue.end();i++){
       if((*i).first==_name)
         result=(*i).second;
     }
     return result;
}

void file_manager::add_file(std::string _name){
  std::unique_lock<std::mutex>lock(m_mutex);
  if(exist(_name)!=-1)
    return;
  m_files[m_filenum]=(file_p)new file_manager::file_s(_name);
  m_files[m_filenum]->file_stream=new std::fstream(_name.c_str());
  m_files[m_filenum]->name=_name;
  m_files[m_filenum]->fd=0;
  m_filenum++;
  m_catalogue.insert(std::pair<std::string,int>(_name,m_filenum-1));
}

bool file_manager::open_file(const std::string& _name,file_s::access _access){
  auto result=m_catalogue.find(_name);
  if(result==m_catalogue.end()){
    std::cout<<"there isn't the file"<<std::endl;
     return false;
  }
  std::unique_lock<std::mutex>lock(m_mutex);   
  int index=result->second;
  return open_file(index,_access);
}

bool file_manager::close_file(const std::string& _name){
  auto result=m_catalogue.find(_name);
  if(result==m_catalogue.end())
     return false;
  int index=result->second;
  return close_file(index);
}

bool file_manager::open_file(const uint& index,file_s::access _access){
     switch (_access){
     case 0:{
       m_files[index]->file_stream->open(m_files[index]->name.c_str(),std::ios::in);
       break;
     }
     case 1:{
       m_files[index]->file_stream->open(m_files[index]->name.c_str(),std::ios::out);
       break;
     }
     case 3:{
       m_files[index]->file_stream->open(m_files[index]->name.c_str(),std::ios::out | std::ios::in | std::ios::binary);
       break;
     }
     case 4:{
       m_files[index]->file_stream->open(m_files[index]->name.c_str(),std::ios::in);
       break;
     }
     case 5:{
       m_files[index]->file_stream->open(m_files[index]->name.c_str(),std::ios::out);
       break;
     }
     default:
       break;
  }
     if(!(m_files[index]->file_stream->is_open())){
       std::cout<<"system error"<<std::endl;
       return false;
     }
     m_files[index]->opened=true;
     return true;
}

bool file_manager::close_file(const uint& index){
    std::unique_lock<std::mutex>lock(m_mutex);
    if(!m_files[index]->opened)
      return false;
    m_files[index]->file_stream->close();
    return true;
}

bool file_manager::rw_file(const std::string& _filename,std::string& _buffer,io_operate type){
    auto result=m_catalogue.find(_filename);
    if(result==m_catalogue.end())
       return false;
    int index=result->second;
    if(m_files[index]->opened)
      close_file(index);
    open_file(index,file_s::access::rw);    
    std::fstream& stream=*(m_files[index]->file_stream);
    if(!stream.is_open()){
       std::cout<<_filename<<" can not be opened,in "<<__LINE__<<" line"<<std::endl;
       return false;
    }
    switch(type){
      case 0:{     
             std::string line="";
             while(!stream.eof()){
               std::getline(stream,line);
               _buffer.append(line+"\n");
               }
            break;}
      case 1:{
             stream.clear();
             stream.write(_buffer.c_str(),strlen(_buffer.c_str()));
             stream.flush();
             break;}      
    }
    stream.close();
    return close_file(index);  
}


void file_manager::delete_file(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(!m_files)
       return;
    for(int i=0;i<m_maxnum;i++){
      if(m_files[i]){
        delete(m_files[i]);
        m_files[i]=nullptr;
      }
    }
    delete[] m_files;
    m_files=nullptr;   
}

std::fstream* file_manager::get_stream(const std::string& _name){
    auto result=m_catalogue.find(_name);
    if(result==m_catalogue.end())
      return nullptr;
    int index=result->second;  
    return (m_files[index]->file_stream);
}