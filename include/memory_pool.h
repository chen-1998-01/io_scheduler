#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

//设计思想：*memory_block设置用来处理大块内存的分配和回收:*
         //*小块内存处理时，设置了内存池和缓存队列，首先在内存池中预留一些内存块，
         //当需要分配时，首先访问空闲队列是否有符合大小的，如果有的话直接调用，
         //如果没有则需要访问内存池，若内存池不够的话，则需要系统分配*         



#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstdint>
#include<cstring>
#include<mutex>
#include<vector>
#include<memory>

class memory_block{
    friend class memory_manager;
    private:
       size_t m_size;
       size_t m_num;  
       std::vector<void*>m_freememory;  
    public:
       typedef std::shared_ptr<memory_block> _block_pool;
       memory_block();
       memory_block(const size_t size,const uint num);
       ~memory_block();
    protected:
        void expand(uint num=0);   
    public:  
        void* allocate_memory();
        void recory_memory(void*& memory);
        uint get_length()const;
        uint get_size()const;
};



class memory_manager{
  public:
    static uint max_size; 
    static uint node_size;
    static uint node_num;
    static uint list_num;
  private:
      memory_block m_block;//用来分配大块内存
      std::mutex m_mutex;
  private:
      static memory_manager* m_manager;
      memory_manager();
      struct memory_node{
        memory_node* next;
        char data[1];
      };
      memory_node* m_free_list[16];//存储空闲的内存队列,用来分配小块内存
      char* m_begin;
      char* m_end;//标记内存池中内存块的起点与终点位置
      std::vector<char*> m_MemoryPool;
      size_t roundup(const size_t size)const;//将size扩容为结点大小的整数倍，返回扩容后的大小
      size_t index(const size_t size)const;//计算能够容纳size大小的最小索引，索引从0开始计数
      std::pair<void*,uint> allocate(const size_t size,const uint num);
      void* fill(const size_t size,const uint num=0);//填充内存池
  public:
       static  memory_manager* GetMemoryManger();   
       ~memory_manager();
  public:

    template<typename T,typename... args>
     T* new_object(args&& ... arg ){
        size_t size=sizeof(T);
        if(size>max_size){
           void* memory=m_block.allocate_memory();
           T* object=new(memory) T(std::forward<args>(arg)...);
           return object;}
    else{
        std::unique_lock<std::mutex>lock(m_mutex);
        memory_node** free_node=&(m_free_list[index(size)]);
        if((*free_node)==nullptr){
           void* head;
           head=fill(size);
           T* newobject=new(head) T(std::forward<args>(arg)...);
           return newobject;
        }//当空闲的队列没有符合大小的结点时，从内存池里调用内存块
        memory_node* next_node=(*free_node)->next;
        T* newobject=new(*free_node) T(std::forward<args>(arg)...);
        m_free_list[index(size)]=next_node;
        return newobject;   
      }//当空闲的队列里面有符合大小的结点时，直接取用
}

  template<typename T>
     T* default_new_object(){
         size_t size=sizeof(T);
         if(size>max_size){
           void* memory=m_block.allocate_memory();
           T* newobject=new(memory) T;
           return newobject;
      }
      {
        std::unique_lock<std::mutex>lock(m_mutex);
        memory_node** free_node=&(m_free_list[index(size)]);
        if((*free_node)==nullptr){
           void* head;
           head=fill(size);
           T* newobject=new(head) T();
           return newobject;
        }
        memory_node* next_node=(*free_node)->next;
        T* newobject=new(*free_node) T();
        m_free_list[index(size)]=next_node;
        return newobject;
     }
   }

     template<typename T>
     void delete_object(T* &object){
      if(!object)
        return;
      if(sizeof(object)>max_size){
        object->~T();
        m_block.recory_memory((void*&)object);
        return;
       }
      memory_node* garbage=(memory_node*)object;
      {
        std::unique_lock<std::mutex>lock(m_mutex);
        object->~T();
        memory_node** free_node=&m_free_list[index(sizeof(garbage))];
        garbage->next=*free_node;
        m_free_list[index(sizeof(garbage))]=garbage;
        object=nullptr;
      }//当大小不超过128b时，将其回收到空闲队列中
    }


     template<typename T>
     T* malloc_memory(const size_t size){
        void* memory=m_block.allocate_memory();
        if(sizeof(memory)<size)
           return nullptr;
        T** object=&(T*)memory;
        return *object;              
      }


    template<typename T>
    void recycle_memory(T* &object){
         if(!object)
           return;
         object->~T();
         std::unique_lock<std::mutex>lock(m_mutex);
         m_block.recory_memory((void*&)(object));
         object=nullptr;  
       }       
};
 
  

#endif