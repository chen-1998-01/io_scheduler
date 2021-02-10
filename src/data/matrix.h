#ifndef _MATRIX_H_
#define _MATRIX_H_

#include<iostream>
#include<cmath>
#include<algorithm>

template<typename T>
class matrix{
   private:
       int m_rows,m_collumns;
       T** m_data;
   private:
        matrix()=delete;
        void init();
        void set_data(const T& value,const int& row,const int& collum);
   public:
       matrix(int rows,int collumns);
       matrix(int rows,int collumns,int value);
       virtual ~matrix();
       matrix(const matrix& obj);
       matrix<T>& operator=(const matrix& obj);
       matrix<T>& operator=(T* data);
       matrix<T>& operator+=(const matrix& obj);
       matrix<T>& operator-=(const matrix& obj);
       matrix<T>& operator*=(const matrix& obj);
       bool operator==(const matrix& obj);
    public:
       void show()const;
       void get_rows()const;
       void get_collumns()const;
       T det();
       T get_data(const int& row,const int& collumn)const;
    public:
        static matrix<T> transposition(const matrix& obj);
        static matrix<T>& unit_matrix(const int& size);               
};


template<typename T>
void matrix<T>::init(){
    m_data=new T*[m_rows];
    for(int i=0;i<m_rows;i++)
       m_data[i]=new T[m_collumns];
    for(int i=0;i<m_rows;i++)
       for(int j=0;j<m_collumns;j++)
          m_data[i][j]=0;   
}

template<typename T>
matrix<T>::matrix(int rows,int collumns):m_rows(rows),m_collumns(collumns){
         init();
}

template<typename T>
matrix<T>::matrix(int rows,int collumns,int value):m_rows(rows),m_collumns(collumns){
         init();
         for(int i=0;i<m_rows;i++)
            for(int j=0;j<m_collumns;j++)
                m_data[i][j]=value;
}

template<typename T>
matrix<T>::matrix(const matrix& obj){
  m_rows=obj.get_rows();
  m_collumns=obj.get_collumns();
  init();
  for(int i=0;i<m_rows;i++)
     for(int j=0;j<m_collumns;j++){
        T data=obj.get_data(i,j);
        m_data[i][j]=data;
     }
}

template<typename T>
void matrix<T>::get_rows()const{
  return m_rows;
}

template<typename  T>
void matrix<T>::get_collumns()const{
  return m_collumns;
}

template<typename T>
T matrix<T>::get_data(const int& row,const int& collumn)const{
    return m_data[row][collumn];
}

template<typename T>
void matrix<T>::set_data(const T& value,const int& row,const int& collumn){
    m_data[row][collumn]=value;
}

template<typename T>
void matrix<T>::show()const{
     for(int i=0;i<m_rows;i++){
       for(int j=0;j<m_collumns;j++){
         std::cout<<m_data[i][j];
       }
       std::cout<<std::endl;
     }
}

template<typename T>
matrix<T>& matrix<T>::operator=(const matrix& obj){
     for(int i=0;i<m_rows;i++)
        for(int j=0;j<m_collumns;j++)
          m_data[i][j]=obj.get_data(i,j);
     return *this;     
} 

template<typename T>
matrix<T>& matrix<T>::operator=(T* data){
    for(int i=0;i<m_rows;i++)
       for(int j=0;j<m_collumns;j++)
          m_data[i][j]=*(data+i*m_collumns+j);
    return *this;          
}

template<typename T>
matrix<T>& matrix<T>::operator+=(const matrix& obj){
    for(int i=0;i<m_rows;i++)
       for(int j=0;j<m_collumns;j++)
           m_data[i][j]+=obj.get_data(i,j);
     return *this;      
}

template<typename T>
matrix<T>& matrix<T>::operator-=(const matrix& obj){
 for(int i=0;i<m_rows;i++)
       for(int j=0;j<m_collumns;j++)
           m_data[i][j]-=obj.get_data(i,j);
     return *this;      
}

template<typename T>
bool matrix<T>::operator==(const matrix& obj){
  bool value=true;
  for(int i=0;i<m_rows;i++)
     for(int j=0;j<m_collumns;j++){
       if(m_data[i][j]!=obj.get_data(i,j))
          value=false;
     }
  return value;
} 

template<typename T>
matrix<T>& matrix<T>::operator*=(const matrix& obj){
   if(m_rows!=m_collumns)
      std::abort();
   for(int i=0;i<m_rows;i++)
       for(int j=0;j<m_collumns;j++){
           T value=0;
           for(int k=0;k<m_collumns;k++)
               value+=(this->m_data[i][k])*obj.get_data(k,i);
            this->m_data[i][j]=value;              
       }         
   return *this;  
}

template<typename T>
T matrix<T>::det(){
    if(m_rows!=m_collumns)
       std::abort();
    T value=0;
    return value;   
}

template<typename T>
matrix<T>& matrix<T>::unit_matrix(const int& size){
   matrix<T>* new_matrix=new matrix<T>(size,size);
   for(int i=0;i<size;i++)
       new_matrix->set_data((T)1.0,i,i);
   return *new_matrix;    
}

template<typename T>
matrix<T> matrix<T>::transposition(const matrix& obj){
    int rows=obj.get_rows();
    int columns=obj.get_collumns();
    matrix<T> new_matrix(columns,rows);
    for(int i=0;i<new_matrix.get_rows();i++)
        for(int j=0;j<new_matrix.get_collumns();j++){
            new_matrix[i][j]=obj.get_data(j,i);
        }    
    return new_matrix;            
}

template<typename T>
matrix<T>::~matrix(){
  for(int i=0;i<m_rows;i++){
    delete(m_data[i]);
    m_data[i]=NULL;
  }
  delete[]m_data;
  m_data=NULL;
}

#endif