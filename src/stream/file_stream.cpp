#include"file_stream.h"
#include"../manager/logger.h"

file::file(int num):m_maxnum(num),m_filenum(0){
   m_files=new file_p[m_maxnum];
   for(int i=0;i<m_maxnum;i++)
     m_files[i]=nullptr;
}

file::~file(){
   if(m_files){
     delete_file();
   }
}

bool file::exist(const std::string& _name){
     bool result=false;
     for(auto i:m_directionary){
       if(i.first==_name)
         result=true;
     }
     return result;
}

void file::add_file(const std::string& _name){
  std::unique_lock<std::mutex>lock(m_mutex);
  if(exist(_name))
    return;
  m_files[m_filenum]=(file_p)new file::file_s(_name);
  m_filenum++;
  std::pair<std::string,int>newpair(_name,m_filenum-1);
  m_directionary.insert(newpair);
}

bool file::open_file(const std::string& _name,file_s::pattern _pattern){
  auto result=m_directionary.find(_name);
  if(result==m_directionary.end())
     return false;
  int index=result->second;
  open_file(index,_pattern);
}

bool file::close_file(const std::string& _name){
  auto result=m_directionary.find(_name);
  if(result==m_directionary.end())
     return false;
  int index=result->second;
  close_file(index);
}

bool file::open_file(const int& index,file_s::pattern _pattern){
     std::unique_lock<std::mutex>lock(m_mutex);
     switch (_pattern)
     {
     case 0:{
       m_files[index]->file_stream.open(m_files[index]->name.c_str(),std::ios::in | std::ios::out);
       break;
     }
     case 1:{
       m_files[index]->file_stream.open(m_files[index]->name.c_str(),std::ios::in);
       break;
     }
     case 2:{
       m_files[index]->file_stream.open(m_files[index]->name.c_str(),std::ios::out);
       break;
     }
     default:
       break;
     }
     if(!m_files[index]->file_stream.is_open()){
       return false;
     }
     m_files[index]->opened=true;
     return true;
}

bool file::close_file(const int& index){
    std::unique_lock<std::mutex>lock(m_mutex);
    if(!m_files[index]->opened)
      return false;
    m_files[index]->file_stream.close();
    return true;
}

void file::read_file(const std::string& _name,std::string& _data){
    auto result=m_directionary.find(_name);
    if(result==m_directionary.end())
       return;
    int index=result->second;
    if(m_files[index]->opened)
      close_file(index);
    open_file(index);    
    std::fstream& in=m_files[index]->file_stream;
    if(!in.is_open()){
       std::cout<<_name<<" can not be opened,in "<<__LINE__<<" line"<<std::endl;
       return;
    }     
    std::string line="";
    while(!in.eof()){
      std::getline(in,line);
      _data.append(line+"\n");
    }
    in.close();
    open_file(index);  
}

void file::delete_file(){
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

std::fstream* file::get_stream(const std::string& _name){
  auto result=m_directionary.find(_name);
  if(result==m_directionary.end())
    return nullptr;
  int index=result->second;  
  return &(m_files[index]->file_stream);
}