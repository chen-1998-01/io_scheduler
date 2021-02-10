#ifndef _BINARYTREE_H_
#define _BINARYTREE_H_

#include"key_value.h"
#include<vector>
#include<queue>
#include<map>


template<typename key,typename value>
class binary_tree{
  private:
       node<key,value>* m_root;
       size_t m_num;
       std::vector<node<key,value>*>m_vectors;
  public:
        binary_tree();
        binary_tree(std::vector<node<key,value>* >& nodes);
        ~binary_tree();
  protected:
        bool exist(const key& _key)const;
        node<key,value>*& Insert(node<key,value>*& newnode,node<key,value>** position);
        node<key,value>* Delete(node<key,value>* deletednode,node<key,value>* position);
        node<key,value>* search(const key& _key);
        node<key,value>* floor(node<key,value>* postion);
        node<key,value>* ceiling(node<key,value>* position);
        node<key,value>* rebuild(node<key,value>* position);
        void free(node<key,value>* position);
        node<key,value>* free_node(node<key,value>* position);
        int depth(node<key,value>*& position);
        void print();
        void level_print(node<key,value>*& position);
  public:
        void Insert(const key& _key,const value& _value);
        void Delete(const key& _key,value _value=0);
        void free();
        int depth();
        void level_print();               
};

template<typename key,typename value>
binary_tree<key,value>::binary_tree():
m_root(NULL),m_num(0){

}


template<typename key,typename value>
binary_tree<key,value>::binary_tree(std::vector<node<key,value>* >& nodes):
m_root(nullptr),m_num(0){
  int size=nodes.size();
  for(int i=0;i<size;i++)
     m_root=Insert(nodes[i],&m_root);  
}
  
template<typename key,typename value>
binary_tree<key,value>::~binary_tree(){
     free();
}

template<typename key,typename value>
node<key,value>*& binary_tree<key,value>::Insert(node<key,value>*& newnode,node<key,value>** position){
    if((newnode)==nullptr)
       std::cout<<"error"<<std::endl;
    if((*position)==nullptr){
       *position=newnode;
       m_num++;
       return *position;
    } 
    if((newnode)->search>(*position)->search){
      (*position)->next[1]=Insert(newnode,&((*position)->next[1]));
    }
    else if((newnode)->search<(*position)->search){
      (*position)->next[0]=Insert(newnode,&((*position)->next[0]));
    }
    else{
       (*position)->data=(newnode)->data;
    }
     return *position;       
}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::Delete(node<key,value>* deletednode,node<key,value>* position){
     if(position==nullptr)
        return nullptr;
     if(deletednode->search>position->search)
        position->next[1]=Delete(deletednode,position->next[1]);
     else if(deletednode->search<position->search)
        position->next[0]=Delete(deletednode,position->next[0]);
     else{
         return rebuild(position);
     }         
}

template<typename key,typename value>
void binary_tree<key,value>::Insert(const key& _key,const value& _value){
    node<key,value>* new_node=new node<key,value>(_key,_value,2);
    m_root=Insert(new_node,&m_root);
}

template<typename key,typename value>
void binary_tree<key,value>::Delete(const key& _key,value _value){
        node<key,value>* deletednode=search(_key);
        if(!deletednode)
           return;
        m_root=Delete(deletednode,m_root);   
}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::floor(node<key,value>* position){
    if(position==nullptr)
       return nullptr;
    node<key,value>* temp=position->next[1];
    if(temp==nullptr)
      return position;
    while(temp->next[0])
         temp=temp->next[0];
    return temp;          
}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::ceiling(node<key,value>* position){
     if(position==nullptr)
       return nullptr;
     node<key,value>* temp=position->next[1];
     if(temp==nullptr)
        return position;
     while(temp->next[1])
        temp=temp->next[1];
     return temp;             
}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::rebuild(node<key,value>* position){
     if(position->next[0]==nullptr && position->next[1]==nullptr){
          delete(position);
          position=NULL;
          return nullptr;
     }        
     node<key,value>* new_node=nullptr;
     if(position->next[1]==nullptr){
          new_node=ceiling(position);
          node<key,value>* temp=position->next[0];
          while(temp->next[1]!=new_node){
               temp=temp->next[1];
          }
          temp->next[1]=nullptr;
          new_node->next[0]=position->next[0];
          new_node->next[1]=position->next[1];
          position->next[0]=nullptr;
          position->next[1]=nullptr;
          delete(position);
          position=NULL;
          return new_node;      
     }
     new_node=floor(position);
     node<key,value>* temp=position->next[1];
     while (temp->next[0]!=new_node){
          temp=temp->next[0];
     }
     temp->next[0]=nullptr;
     new_node->next[0]=position->next[0];
     new_node->next[1]=position->next[1];
     position->next[0]=nullptr;
     position->next[1]=nullptr;
     delete(position);
     position=NULL;
     return new_node;
}

template<typename key,typename value>
void binary_tree<key,value>::free(node<key,value>* position){
     if(position==nullptr)
        return;
     position=free_node(position);

}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::free_node(node<key,value>* position){
     if(position->next[0]){
          position->next[0]=free_node(position->next[0]);
     }
     if(position->next[1]){
          position->next[1]=free_node(position->next[1]);
     }
     delete(position);
     position=NULL;
     return position;
}

template<typename key,typename value>
void binary_tree<key,value>::free(){
     m_root=free(m_root);
}

template<typename key,typename value>
bool binary_tree<key,value>::exist(const key& _key)const{
    bool result=false;
    node<key,value>* temp=m_root;
    if(temp=nullptr)
      return result;
    while (temp) {
      if(temp->search<_key)
         temp=temp->next[1];
      else if(temp->search>_key)
         temp=temp->next[0];
      else{
           result=true;
      }     
    }
    return result;   
}

template<typename key,typename value>
node<key,value>* binary_tree<key,value>::search(const key& _key){
   node<key,value>* temp=m_root;
   if(temp==nullptr)
     return nullptr;
   while(temp){
        if(temp->search<_key)
          temp=temp->next[1];
        else if(temp->search>_key)
          temp=temp->next[0];
        else{
             return temp;
        }     
   }
   return nullptr;  
}

template<typename key,typename value>
int binary_tree<key,value>::depth(){
     return depth(m_root);
}

template<typename key,typename value>
int binary_tree<key,value>::depth(node<key,value>*& position){
     if(position==nullptr)
       return 0;
     int depth_left=depth(position->next[0]);
     int depth_right=depth(position->next[1]);  
     return  (depth_left>depth_right?depth_left:depth_right)+1; 
}

template<typename key,typename value>
void binary_tree<key,value>::print(){
     if(m_vectors.empty())
        return;
     int size=m_vectors.size();
     for(int i=0;i<size;i++){
          std::cout<<m_vectors[i]->search<<","<<m_vectors[i]->data<<"  ";
     }  
}

template<typename key,typename value>
void binary_tree<key,value>::level_print(node<key,value>*& position){
   if(position==nullptr)
     return; 
   if(!m_vectors.empty())
       m_vectors.clear();  
   std::queue<node<key,value>* >temp;
   temp.push(position);
   while(!temp.empty()){
     node<key,value>* head=temp.front();
     if(head->next[0])
        temp.push(head->next[0]);
     if(head->next[1])
        temp.push(head->next[1]);
     temp.pop();    
     m_vectors.push_back(head);        
   }  
}

template<typename key,typename value>
void binary_tree<key,value>::level_print(){
     level_print(m_root);
     print();
}


#endif