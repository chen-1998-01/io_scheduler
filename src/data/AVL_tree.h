#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#include"tree.h"
#include<vector>

template<typename key,typename value>
class alv_tree{
    private:
      node<key,value>* m_root;
      int m_num;
    public:
       alv_tree();
       alv_tree(std::vector<node<key,value>* >& nodes);
      virtual ~alv_tree();
    protected:
        bool balanced(node<key,value>*& position);
        node<key,value>*& lr_roatate(node<key,value>*& position);
        node<key,value>*& rl_roatate(node<key,value>*& position);
        node<key,value>*& rr_roatate(node<key,value>*& position);
        node<key,value>*& ll_roatate(node<key,value>*& position);
        node<key,value>*& Insert(node<key,value>*& newnode,node<key,value>*& position);
        node<key,value>*& Delete(node<key,value>*& deletenode,node<key,value>*& position); 
        node<key,value>* Search(node<key,value>*& position,const key& _key);
        int height(node<key,value>*& position);
        int degree(node<key,value>*& position);
        void adjust(node<key,value>*& position);
    public:
        void Insert(const key& _key,const value& _value);
        void Delete(const key& _key);
        int height();
        int nodes_num()const;
        bool exist(const key& _key);
        value& Search(const key& _key);         
};

template<typename key,typename value>
alv_tree<key,value>::alv_tree():m_root(nullptr){
   
}

template<typename key,typename value>
alv_tree<key,value>::alv_tree(std::vector<node<key,value>* >& nodes){
     int size=nodes.size();
     for(int i=0;i<size;i++){
      m_root=Insert(nodes[i],m_root);
     }
}

template<typename key,typename value>
alv_tree<key,value>::~alv_tree(){
     if(m_root){
     delete(m_root);
     m_root=NULL;
   }
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::lr_roatate(node<key,value>*& position){
     node<key,value>* temp_root=position->next[0];
     position->next[0]=rr_roatate(temp_root);
     return ll_roatate(position);
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::rl_roatate(node<key,value>*& posiition){
       node<key,value>* temp_root=posiition->next[1];
       posiition->next[1]=ll_roatate(temp_root);
       return rr_roatate(posiition);
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::ll_roatate(node<key,value>*& position){
    node<key,value>* new_root=position->next[0];
    position->next[0]=new_root->next[1];
    new_root->next[1]=position;
    return new_root;
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::rr_roatate(node<key,value>*& position){
    node<key,value>* new_root=position->next[1];
    position->next[1]=new_root->next[0];
    new_root->next[0]=position;
    return new_root;
}

template<typename key,typename value>
bool alv_tree<key,value>::balanced(node<key,value>*& position){
    int _value=fabs(degree(position));
    if(_value<=1)
        return true;
     return false;    
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::Insert(node<key,value>*& newnode,node<key,value>*& position){
      if(position==nullptr){
        position=newnode;
        return position;
      }
      if(newnode->search<position->search)
        position->next[0]=Insert(newnode,position->next[0]);
      else if(newnode->search>position->search)
        position->next[1]=Insert(newnode,position->next[1]);
      else{
        position->data=newnode->data;
        delete(newnode);
        newnode=nullptr;  
      }
      adjust(position);
      return position;            
}

template<typename key,typename value>
void alv_tree<key,value>::Insert(const key& _key,const value& _value){
      node<key,value>* newnode=new node<key,value>(_key,_value);
      m_root=Insert(newnode,m_root);
}

template<typename key,typename value>
node<key,value>*& alv_tree<key,value>::Delete(node<key,value>*& deletenode,node<key,value>*& position){
     free(deletenode);
     deletenode=NULL;
     adjust(m_root);
}

template<typename key,typename value>
void alv_tree<key,value>::Delete(const key& _key){
    node<key,value>*& deletenode=Search(m_root,_key);
    if(!deletenode)
       return;
    m_root=Delete(deletenode,m_root);      
}

template<typename key,typename value>
int alv_tree<key,value>::height(node<key,value>*& position){
    if(position==nullptr)
       return 0;
    int left_height=height(position->next[0]);
    int right_height=height(position->next[1]);
    return (left_height>right_height?left_height:right_height)+1;   
}

template<typename key,typename value>
int alv_tree<key,value>::height(){
  return height(m_root);
}

template<typename key,typename value>
int alv_tree<key,value>::degree(node<key,value>*& position){
     if(position==nullptr)
        return 0;
     int degree=degree(position->next[0])-degree(position->next[1]);
     position->balance_degree=degree;
     return degree;       
}

template<typename key,typename value>
int alv_tree<key,value>::nodes_num()const{
      return m_num;
}


template<typename key,typename value>
void alv_tree<key,value>::adjust(node<key,value>*& position){
      if(balanced(position))
         return;
      if(degree(position)>1){
        if(degree(position->next[0])>0){
          position=ll_roatate(position);
        }
        else if(degree(position->next[0])<=0){
          position=lr_roatate(position);
        }
      } 
      else if(degree(position)<-1){
        if(degree(position->next[1])<=0){
          position=rr_roatate(position);
        }
        else if(degree(position->next[1])>0){
          position=rl_roatate(position);
        }
      }  
}

template<typename key,typename value>
bool alv_tree<key,value>::exist(const key& _key){
       bool result=false;
       node<key,value>* temp_node=m_root;
       while(temp_node){
         if(_key>temp_node->search)
             temp_node=temp_node->next[1];
         else if(_key<temp_node->search)
              temp_node=temp_node->next[0];
         else{
              result=true;
              break;
          }        
       } 
       return result;
}

template<typename key,typename value>
node<key,value>* alv_tree<key,value>::Search(node<key,value>*& position,const key& _key){
       node<key,value>* result=nullptr;
       node<key,value>* precursor=position;
       while(precursor){
         if(_key<percursor->search)
            precursor=percursor->next[0];
         else if(_key>percursor->search)
            precursor=percursor->next[1];
          else{
            result=precursor;
            break;
          }      
       }
       return result;
}

template<typename key,typename value>
value& alv_tree<key,value>::Search(const key& _key){
    node<key,value>*& result=Search(m_root,_key);
    if(!result)
       return 0;
    return result->data;   
}


#endif