#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstdint>
#include<cstring>
#include<mutex>
#include<vector>
#include<memory>

class memory_pool{
    friend class class_memory_pool;
    private:
       std::mutex m_mutex;
       size_t m_size;
       size_t m_num;  
       std::vector<void*>m_freememory;  
    public:
       typedef std::shared_ptr<memory_pool> _block_pool;
       memory_pool();
       memory_pool(const size_t size,const uint num);
       ~memory_pool();
    protected:
        void expand(uint num);   
    public:  
        void* allocate_memory();//分配内存
        void recory_memory(void* &memory);//回收内存
        uint get_length()const;
        uint get_size()const;
};



class class_memory_pool{
  public:
    static uint max_size; 
    static uint node_size;
    static uint node_num;
    static uint list_num;
    typedef std::shared_ptr<class_memory_pool> _pool;
  private:
      memory_pool m_pool;
      std::mutex m_mutex;
  private:
      union memory_node{
        memory_node* next;
        char data[1];
      };
      typedef memory_node* node_pointer;
      memory_node* m_free_list[16];//存储空闲的内存队列
      char* m_begin;
      char* m_end;//标记内存池中内存块的起点与终点位置
      std::vector<char*> m_malloc_memory;
      size_t roundup(const size_t size);
      size_t index(const size_t size);
      std::pair<void*,uint> allocate(const size_t size,const uint num);
       void* fill(const size_t size,const uint num);
  public:
       class_memory_pool();
       class_memory_pool(const size_t size,const uint num);     
       ~class_memory_pool();
  public:
    template<typename T,typename... args>
     T* new_class(args&& ... arg ){
        size_t size=sizeof(T);
        if(size>max_size){
           void* memory=malloc(size);
           T* object=new(memory) T(std::forward<args>(arg)...);
           return object;}
   {
      std::unique_lock<std::mutex>lock(m_mutex);
      memory_node** free_node=&(m_free_list[index(size)]);
      if((*free_node)==nullptr){
         void* head;
         head=fill(roundup(size),0);
         T* newobject=new(head) T(std::forward<args>(arg)...);
         std::cout<<"system alloacate memory"<<std::endl;
         return newobject;
      }//当空闲的队列没有符合大小的结点时，从内存池里调用内存块
      memory_node* next_node=(*free_node)->next;
      T* newobject=new(*free_node) T(std::forward<args>(arg)...);
      m_free_list[index(size)]=next_node;
      std::cout<<"memory pool alloacate memory"<<std::endl;
      return newobject; 
   }//当空闲的队列里面有符合大小的结点时，直接取用
}

  template<typename T>
     T* default_new_class(){
         size_t size=sizeof(T);
         if(size>max_size){
           void* memory=malloc(size);
           T* newobject=new(memory) T;
           return newobject;
        }
      {
        std::unique_lock<std::mutex>lock(m_mutex);
        memory_node** free_node=&(m_free_list[index(size)]);
        if((*free_node)==nullptr){
           void* head;
           head=fill(roundup(size),0);
           T* newobject=new(head) T();
           std::cout<<"system alloacate memory"<<std::endl;
           return newobject;
        }
        memory_node* next_node=(*free_node)->next;
        T* newobject=new(*free_node) T();
        m_free_list[index(size)]=next_node;
        std::cout<<"memory pool alloacate memory"<<std::endl;
        return newobject;
     }
   }

     template<typename T>
     void delete_class(T* &object){
        if(!object)
        return;
     if(sizeof(object)>max_size){
       object->~T();
       free(object);
       object=nullptr;
       return;
     }
     //当析构的大小超过128b时，直接free
     memory_node* garbage=(memory_node*)object;
     {
       std::unique_lock<std::mutex>lock(m_mutex);
       object->~T();
       memory_node** free_node=&m_free_list[index(sizeof(garbage))];
       garbage->next=*free_node;
       m_free_list[index(sizeof(garbage))]=garbage;
       object=nullptr;
       std::cout<<"recycle into the memory pool"<<std::endl;
     }
     //当大小不超过128b时，将其回收到空闲队列中
}
     

     template<typename T>
     T* malloc_memory(const size_t size){
        void* memory=m_pool.allocate_memory();//分配
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
         m_pool.recory_memory((void*&)(object));//回收
         object=nullptr;  
       }       
};
 
  

#endif