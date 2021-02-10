#include"../src/stream/file_stream.h"

int main(int argc,char* argv[]){
   file* t_files=new file(2);
   std::string filename="hello.txt";
   t_files->add_file(filename);
   std::string data;
   t_files->read_file(filename,data);
   std::cout<<data<<std::endl;
   std::string copy=data;
   std::cout<<copy;
}