#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include<functional>
#include<iostream>
#include<vector>


template<typename key,typename value>
struct hashnode{
  key _key;
  value _value;
  hashnode* next;
  hashnode(const key& k,const value& v):_key(k),_value(v),next(nullptr){};
};

template<typename key,typename value>
class hashtable{
  private:
       int m_size;
       hashnode<key,value>** m_datas;
       std::function<int(key)> m_function;
  private:
       void resize(int size);        
  public:
       typedef hashnode<key,value>* data_type;
       hashtable(std::function<int(key)>& func,int size=1);
       ~hashtable();
  public:
       bool Insert(const key& _key,const value& _value);
       bool Erease(const key& _key,const value& _value);
       bool Search(const key& _key,std::vector<value>& vectors);     
};

template<typename key,typename value>
hashtable<key,value>::hashtable(std::function<int(key)>& func,int size):m_size(size){
   m_function=func;
   m_datas=new data_type[m_size];
   for(int i=0;i<m_size;i++)
     m_datas[i]=nullptr;
}

template<typename key,typename value>
hashtable<key,value>::~hashtable(){
    if(m_datas){
         for(int i=0;i<m_size;i++){
              if(m_datas[i]){
                   delete(m_datas[i]);
                   m_datas[i]=nullptr;
              }
         }
         delete[] m_datas;
         m_datas=nullptr;
    }
}

template<typename key,typename value>
void hashtable<key,value>::resize(int size){
     data_type* newdata=new data_type[size];
     for(int i=0;i<m_size;i++)
        newdata[i]=m_datas[i];
     for(int i=m_size;i<size;i++)
        newdata[i]=nullptr;
     m_datas=newdata;
     m_size=size;     
}

template<typename key,typename value>
bool hashtable<key,value>::Insert(const key& _key,const value& _value){
   data_type newnode=new hashnode<key,value> (_key,_value);
   if(!newnode)
      return false;
   int index=m_function(_key);
   std::cout<<index<<std::endl;
   if(m_size<=index)
      resize(index*2);
   if(m_datas[index]==nullptr){
        m_datas[index]=newnode;
        return true;
   }
   data_type pre_node=m_datas[index];      
   while(pre_node->next)
       pre_node=pre_node->next;
   pre_node->next=newnode;
   return true;    
}


template<typename key,typename value>
bool hashtable<key,value>::Erease(const key& _key,const value& _value){
     int index=m_function(_key);
     if(index>=m_size || m_datas[index]==nullptr)
       return false;
     if(m_datas[index]->value==_value){
          data_type next_data=m_datas[index]->next;
          delete(m_datas[index]);
          m_datas[index]=next_data;
          return true;
     }
     data_type pre_node=m_datas[index];
     bool flage=false;
     while(pre_node->next){
          if(pre_node->next->_value==_value){
               data_type next_data=pre_node->next->next;
               delete(pre_node->next);
               pre_node->next=next_data;
               flage=true;
               break;
          }
          pre_node=pre_node->next;
     }
     return flage;    
}

template<typename key,typename value>
bool hashtable<key,value>::Search(const key& _key,std::vector<value>& vectors){
   vectors.clear();
   int index=m_function(_key);
   if(index>=m_size || m_datas[index]==nullptr)
     return false;
   vectors.push_back(m_datas[index]->_value);
   data_type pre_node=m_datas[index];
   while(pre_node->next){
        vectors.push_back(pre_node->next->_value);
        pre_node=pre_node->next;  
   }
   return true;  
}

#endif