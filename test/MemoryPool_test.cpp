#include"../include/memory_pool.h"

struct object{
    int x;
    int y;
    object():x(0),y(0){};
};


int main(int argc,char* argv[]){
     memory_manager* MemoryPool_test = memory_manager::GetMemoryManger(); 
     for(int i=0;i<1000;i++){
       object* object_test=MemoryPool_test->new_object<object>();
       MemoryPool_test->delete_object<object>(object_test);
     }  
}