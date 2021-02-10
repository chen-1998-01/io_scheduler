#ifndef _ARRAY_H_
#define _ARRAY_H_

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<ctype.h>
#include<cmath>
#include<algorithm>
#include<unistd.h>
#include<memory>
#include<memory.h>
#include<exception>
#include"../../stream/file_stream.h"

template<typename T>
class array{
  protected:
     T* m_data;
     size_t m_size;
     uint m_num;     
  public:
      array(const uint num=0);
      array(const T* data,const uint num,bool copy);
      array(const array<T>& obj);
      virtual ~array();
      T& operator[](const uint index)const;
      T& at(const uint index)const;
      T* begin();
      T* end();
      void set(const uint& index,const T& value);
      T& get(const uint& index)const;
      bool insert(uint index,const T& value);
      bool insert(const T& value);
      bool erease(uint index);
     void sort();
     virtual void print();
     size_t get_size()const{return m_size;}
   protected:
      virtual std::istream& read(std::istream& in);
      virtual std::ostream& write(std::ostream& out);       
};

template<typename T>
array<T>::array(const uint num):m_num(num){
   m_size=sizeof(T);
   if(m_num){
    m_data=new T[m_num];
    for(int i=0;i<m_num;i++)
       m_data[i]=0;
   }   
}

template<typename T>
array<T>::array(const T* data,const uint num,bool copy):
m_num(num){
   m_size=sizeof(T);
   if(copy==false){
      m_data=data;
   }
   else if(copy==true){
     m_data=new T[m_num];
     for(int i=0;i<m_num;i++)
        m_data[i]=data[i];
   }
}

template<typename T>
array<T>::array(const array<T>& obj){
  m_num=obj.get_size();
  m_size=sizeof(T);
  m_data=new T[m_num];
  for(int i=0;i<m_num;i++)
     m_data[i]=obj.get(i);
}

template<typename T>
array<T>::~array(){
   if(m_data){
     delete(m_data);
     m_data=NULL;
     m_num=0;
     m_size=0;
   }
}

template<typename T>
T& array<T>::operator[](const uint index)const{
    return m_data[index];
}

template<typename T>
T& array<T>::at(const uint index)const{
     return m_data[index];
}

template<typename T>
T* array<T>::begin(){
   return m_data;
}

template<typename T>
T* array<T>::end(){
    return (m_data+m_num);
}

template<typename T>
bool array<T>::insert(const T& value){
   m_data=(T*)realloc(m_data,(m_num+1)*m_size);
   if(!m_data)
     return false;
   m_data[m_num]=value;
   m_num++;
   return true;  
}

template<typename T>
bool array<T>::insert(uint index,const T& value){
   if(index>m_num)
      return false;
   else if(index==m_num)
       return insert(value);
   T* m_temp=new T[m_num];
   for(int i=0;i<m_num;i++)
      m_temp[i]=m_data[i];       
   m_data=(T*)realloc(m_data,(m_num+1)*sizeof(T));//扩充动态数组
   if(!m_data)
     return false;
   m_num++;  
   for(int i=index+1;i<m_num;i++)
       m_data[i]=m_temp[i-1];
   m_data[index]=value;
   delete[] m_temp;
   m_temp=NULL;    
   return true;
}

template<typename T>
bool array<T>::erease(uint index){
   if(index>=m_num){
     std::cout<<"over the boundarys of the dynamic array,in the "<<__LINE__<<" line"<<std::endl; 
     return false;
   }
   if(!m_data)
      return false;
   m_num--;  
   memmove(m_data+index,m_data+index+1,m_size*(m_num-index));  
   m_data=(T*)realloc(m_data,m_size*m_num);
   std::cout<<"erease successfully"<<std::endl;   
   return true;
}


template<typename T>
void array<T>::sort(){
   std::sort(m_data,m_data+m_num-1);   
}

template<typename T>
std::istream& array<T>::read(std::istream& in){
   in.seekg(0,in.end);
   int length=in.tellg();
   if(m_num<length){
      m_num=length;
      m_data=(T*)realloc(m_data,m_num*m_size);
   }
   for(int i=0;i<m_num;i++)
     m_data[i]=0;
 do{
    try{
       if(!read_stream(in,m_data,m_num))
         break;     
     }
    catch(...){
       std::cout<<"read data error!"<<std::endl;
       return in;
    }
    return in;   
  }while(0);
  return in;
}

template<typename T>
std::ostream& array<T>::write(std::ostream& out){
  out<<"the data address:"<<this<<std::endl;
  if(!write_stream<T*>(out,m_data,m_num))
    std::cout<<"write failed"<<std::endl;  
  out<<std::endl;
  return out;
}

template<typename T>
void array<T>::set(const uint& index,const T& value){
    if(index>=m_num)
      return;
    m_data[index]=value;  
}

template<typename T>
T& array<T>::get(const uint& index)const{
   return m_data[index];
}

template<typename T>
void array<T>::print(){
   if(!m_num)
     return;
   if(m_num>1){  
      for(int i=0;i<m_num-1;i++)
      std::cout<<m_data[i]<<",";
   }
   std::cout<<m_data[m_num-1]<<std::endl;      
}




class Array:public array<int>{
  private:
     char* m_string;
     void data_translate(char*& buf,int*& data,int size);
     bool translated;  
  public:
     Array():m_string(nullptr),translated(false){};
     Array(int num);
     ~Array();
     std::istream& read(std::istream& in)override;
     std::ostream& write(std::ostream& out)override;
     void print()override; 
};

Array::Array(int num){
   m_size=sizeof(int);
   m_num=num;
   m_data=new int[m_num];
   m_string=new char[2*m_num];   
}

Array::~Array(){
  if(m_data){
     delete(m_data);
     m_data=NULL;
     m_num=0;
     m_size=0;
   }   
}

void Array::data_translate(char*& buf,int*& data,int size){
    if(!size)
      return;
    else if(!buf)
       buf=new char[2*size];  
    else if(sizeof(buf)<2*size)
       buf=(char*)realloc(buf,2*size);  
    for(int i=0,j=0;i<size;i++,j+=2){
       buf[j]=data[i]+'0';
       if(i<size-1)
         buf[j+1]=',';
    }
    translated=true;  
}

std::istream& Array::read(std::istream& in){
   in.seekg(0,in.end);
   int length=in.tellg();
   if(m_num<length){
      m_num=length;
      m_data=(int*)realloc(m_data,m_num*m_size);
   }
   for(int i=0;i<m_num;i++)
     m_data[i]=0;
   do{
     try{
        if(!read_stream(in,m_string,m_num))
           break;     
     }
     catch(...){
        std::cout<<"read data error!"<<std::endl;
        return in;
    }
    return in;   
  }while(0);
  return in;
}

std::ostream& Array::write(std::ostream& out){   
    data_translate(m_string,m_data,m_num);
    out<<"the data address:"<<this<<std::endl;
    if(!write_stream<char*>(out,m_string,strlen(m_string)))
     std::cout<<"write failed"<<std::endl;  
    out<<std::endl;
    return out;
}

void Array::print(){
   if(!translated){
     data_translate(m_string,m_data,m_num);
     translated=true;
   }
   std::cout<<m_string<<std::endl;
}

#endif