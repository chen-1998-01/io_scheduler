#include"memory_pool.h"
#include<cassert>
#include<exception>

 uint class_memory_pool::max_size=128;
 uint class_memory_pool::node_num=16;
 uint class_memory_pool::node_size=8;
 uint class_memory_pool::list_num=128/8;

memory_pool::memory_pool():
m_size(1024),m_num(1){

}

memory_pool::memory_pool(const size_t size,const uint num):
 m_size(size),m_num(num){
        assert(m_size>0);
        assert(m_num);
}

memory_pool::~memory_pool(){
  std::unique_lock<std::mutex>lock(m_mutex);
  for(auto i=m_freememory.begin();i!=m_freememory.end();i++){
     if(*i)
      free(*i);
  }
}

void* memory_pool::allocate_memory(){
  std::unique_lock<std::mutex>lock(m_mutex);
  if(m_freememory.empty()){
    expand(0);
  }
  void* memory=m_freememory.back();
  m_freememory.pop_back();
  return memory;
}

void memory_pool::recory_memory(void* &memory){
    std::unique_lock<std::mutex>lock(m_mutex);
    m_freememory.push_back(memory);
}

void memory_pool::expand(uint num){
     if(num==0)
        num=m_num;
     for(uint i=0;i<num;i++){
        void* memory=malloc(m_size);
        m_freememory.push_back(memory);       
     }
}


uint memory_pool::get_length()const{
     return m_size;
}

uint memory_pool::get_size()const{
}


class_memory_pool::class_memory_pool():
m_begin(nullptr),m_end(nullptr),m_pool(){
}

class_memory_pool::class_memory_pool(const size_t size,const uint num):
m_begin(nullptr),m_end(nullptr),m_pool(size,num){
       for(int i=0;i<node_num;i++)
          m_free_list[i]=nullptr;
}

class_memory_pool::~class_memory_pool(){
    m_begin=nullptr;
    m_end=nullptr;
    for(auto i=0;i<16;i++){
       if(m_free_list[i]){
          delete(m_free_list[i]);
          m_free_list[i]=NULL;
       }
    }
    m_malloc_memory.clear();   
}

size_t class_memory_pool::roundup(const size_t size){
  return (size+node_size-1)&~(node_size-1);
}//将size扩容为结点大小的整数倍，返回扩容后的大小

size_t class_memory_pool::index(const size_t size){
  return (size+node_size-1)/node_size-1;
}//计算能够容纳size大小的最小索引，索引从0开始计数


void* class_memory_pool::fill(const size_t size,const uint num){
       uint number=0;
       if(num==0)
          number=node_num;
       char* head;        
       try{
         std::pair<void*,uint>_value=allocate(size,number);
         number=_value.second;
         head=(char*)_value.first;
       }
       catch(std::exception& error){
          std::cout<<error.what()<<std::endl;
       }
       if(number==1)
          return head;

       memory_node* current,next;
       current=(memory_node*)head;
       while(number--){
         current->next=(memory_node*)((char*)current+size);
         current=current->next;
       }
       current->next=nullptr;
       return (memory_node*)head;
}

std::pair<void*,uint> class_memory_pool::allocate(const size_t size,const uint num){
    uint nums=num;
    size_t need_size=size*nums;
    size_t left_size=m_end-m_begin;
    char* head=NULL;
    if(left_size>=need_size){
       head=m_begin;
       m_begin+=need_size;
       return std::make_pair(head,nums);
    }
    else{
      if(left_size>=size){
         nums=left_size/size;
         need_size=size*nums;
         head=m_begin;
         m_begin+=need_size;
         return std::make_pair(head,nums);
      }
      size_t get_size=need_size;
      if(left_size>0){
        assert(index(left_size)>=0);
        memory_node* free_node;
        int _index=index(left_size); 
        free_node=(memory_node*)(m_free_list[_index]);
        ((memory_node*)m_begin)->next=free_node;
        (m_free_list[_index])=(node_pointer)m_begin;//插入新建内存尾部指针
      }
      m_begin=(char*)malloc(need_size);
      m_end=m_begin+need_size;
      m_malloc_memory.push_back(m_begin);
      return allocate(size,num);
    }
}

