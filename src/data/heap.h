#ifndef _HEAP_H_
#define _HEAP_H_
#include"key_value.h"

template<typename key,typename value>
class heap{
  private:
    std::vector<node<key,value> >m_vectors;
  public:
    heap(){};
    heap(std::vector<node<key,value> >&vectors);
    ~heap();
  public:
     void build();
     void insert(node<key,value>& obj);    
     void erease();
     void print();
};

template<typename key,typename value>
heap<key,value>::heap(std::vector<node<key,value> >&vectors){
   for(int i=0;i<vectors.size();i++)
       m_vectors.push_back(vectors[i]);
}

template<typename key,typename value>
heap<key,value>::~heap(){
  m_vectors.clear();
}

template<typename key,typename value>
void heap<key,value>::build(){
    std::make_heap(m_vectors.begin(),m_vectors.end());
}

template<typename key,typename value>
void heap<key,value>::insert(node<key,value>& obj){
     m_vectors.push_back(obj);
     std::push_heap(m_vectors.begin(),m_vectors.end());
}

template<typename key,typename value>
void heap<key,value>::erease(){
      
}

template<typename key,typename value>
void heap<key,value>::print(){
  for(int i=0;i<m_vectors.size();i++)
     std::cout<<i<<":"<<m_vectors[i]<<" ";
  std::cout<<std::endl;  
}

#endif