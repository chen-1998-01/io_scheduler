#include"../../include/memory_pool.h"
#include<cassert>
#include<exception>


memory_block::memory_block():
m_size(1024),m_num(1){

}

memory_block::memory_block(const size_t size,const uint num):
 m_size(size),m_num(num){
        assert(m_size>0);
        assert(m_num);
}

memory_block::~memory_block(){
  for(auto i=m_freememory.begin();i!=m_freememory.end();i++){
     if(*i){
      free(*i);
      *i=nullptr;
     }
  }
}

void* memory_block::allocate_memory(){
  if(m_freememory.empty()){
    expand(0);
  }
  void* memory=m_freememory.back();
  m_freememory.pop_back();
  return memory;
}

void memory_block::recory_memory(void*& memory){
    if(!memory){ 
      m_freememory.push_back(memory);
    }
}

void memory_block::expand(uint num){
     if(num==0)
        num=m_num;
     for(uint i=0;i<num;i++){
        void* memory=malloc(m_size);
        m_freememory.push_back(memory);       
     }
}


uint memory_block::get_length()const{
     return m_num;
}

uint memory_block::get_size()const{
   return m_size;
}


memory_manager* memory_manager::m_manager=nullptr;
uint memory_manager::max_size=128;
uint memory_manager::node_num=16;
uint memory_manager::node_size=8;
uint memory_manager::list_num=128/8;
//分别是默认设置的内存分配节点的内存最大值、节点数量

memory_manager::memory_manager():
m_begin(nullptr),m_end(nullptr),m_block(){
   for(int i=0;i<node_num;i++)
      m_free_list[i]=nullptr;
}

memory_manager* memory_manager::GetMemoryManger(){
   if(!m_manager)
     m_manager=new memory_manager();
   return m_manager;  
}

memory_manager::~memory_manager(){
    m_begin=nullptr;
    m_end=nullptr;
    for(auto i=0;i<16;i++){
       if(m_free_list[i]){
          delete(m_free_list[i]);
          m_free_list[i]=NULL;
       }
    }
    m_MemoryPool.clear();   
}

size_t memory_manager::roundup(const size_t size)const{
  return (size+node_size-1)&~(node_size-1);
}

size_t memory_manager::index(const size_t size)const{
  return (size+node_size-1)/node_size-1;
}


void* memory_manager::fill(const size_t size,const uint num){
       assert(size<=128);
       size_t _size=roundup(size);
       uint number=0;
       if(num==0)
          number=node_num;
       char* head;        
       try{
         std::pair<void*,uint>_value=allocate(_size,number);
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
       while(number){
         current->next=(memory_node*)((char*)current+_size);
         current=current->next;
         number--;
       }
       current->next=nullptr;
       return (memory_node*)head;
}

std::pair<void*,uint> memory_manager::allocate(const size_t size,const uint num){
    uint nums=num;
    size_t need_size=size*nums;
    size_t left_size=m_end-m_begin;
    char* head=NULL;
    if(left_size>=need_size){
       head=m_begin;
       m_begin+=need_size;
       return std::make_pair(head,nums);
    }//内存池能够完全供给，将内存池中的内存块进行供给
    else{
      if(left_size>=size){
         nums=left_size/size;
         need_size=size*nums;
         head=m_begin;
         m_begin+=need_size;
         return std::make_pair(head,nums);
      }//内存池能够供给部分，但不能完全供给，将内存池中内存块进行供应
      size_t get_size=need_size;
      if(left_size>0){
        assert(index(left_size)>=0);
        memory_node* free_node;
        int _index=index(left_size); 
        free_node=(memory_node*)(m_free_list[_index]);
        ((memory_node*)m_begin)->next=free_node;
        (m_free_list[_index])=(memory_node*)m_begin;
      }
      m_begin=(char*)malloc(need_size);
      m_end=m_begin+need_size;
      m_MemoryPool.push_back(m_begin);
      return allocate(size,num);
    }//内存池无法供应任何一块时，先将内存池中的内存块放入到内存队列中，然后系统分配内存
}

