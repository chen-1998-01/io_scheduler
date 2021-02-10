#ifndef _RB_TREE_H_
#define _RB_TREE_H_

#include"key_value.h"
#include<vector>

//采用next[0]代表父亲结点，采用next[1]和next[2]代表左右子树结点
//红黑树的特点：1.每条路径上的黑色节点数量相同
             //2.红色节点不会发生相邻的情况

#ifndef black
#define black 1
#endif

#ifndef red
#define red 0
#endif

template<typename key,typename value>
class rb_tree{
   private:
      node<key,value>* m_root;
      node<key,value>* m_sentiel;
      int m_num;
   public:
      rb_tree();
     virtual ~rb_tree();
   protected:
       void left_roatate(node<key,value>*& position);
       void right_roatate(node<key,value>*& position);
       node<key,value>*& delete_node(node<key,value>*& position);
       void Adjust(node<key,value>*& position);
       node<key,value>*& delete_min(node<key,value>*& position);
       node<key,value>*& delete_max(node<key,value>*& position);
       int height(node<key,value>*& position);
   public:   
        bool Insert(const key& _key,const value& _value);
        bool Delete(const key& _key,value _value);
        bool Search(const key& _key,std::vector<value>&);
        void Print_value(const key& _key);
        int height();
};

template<typename key,typename value>
rb_tree<key,value>::rb_tree():
m_root(nullptr),m_leave(nullptr),m_num(0){
    m_sentiel=new node<key,value>(0,0,0,black);
    m_root=m_sentiel;
}

template<typename key,typename value>
rb_tree<key,value>::~rb_tree(){
   if(m_root){
      delete(m_root);
      m_root=NULL;
   }
   if(m_sentiel){
      delete(m_sentiel);
      m_sentiel=NULL;
   }
}

template<typename key,typename value>
bool rb_tree<key,value>::Insert(const key& _key,const value& _value){
     node<key,value>* newnode=new node<key,value>(_key,_value,3,red);
     if(!newnode)
        return false;
     newnode->next[0]=m_sentiel;   
     newnode->next[1]=m_sentiel;
     newnode->next[2]=m_sentiel;
     node<key,value>* prenode=m_root;
     if(prenode==m_sentiel){
        m_root=newnode;
        m_num++;
        return true;
     }
     node<key,value>* parentnode=nullptr;//用来记录双亲结点
     while(prenode!=m_sentiel){
        parentnode=prenode;
        if(_key<=parentnode->search){
           prenode=prenode->next[1];
        }
        else if(_key>parentnode->search){
           prenode=prenode->next[2];
        }        
     }
      if(_key>parentnode->search){
         parentnode->next[2]=newnode;         
      }
      else if(_key<=parentnode->search){
         parentnode->next[1]=newnode; 
      }
       newnode->next[0]=parentnode;
       Adjust(newnode);
       m_num++;
       return true;
}//采用循环遍历插入结点

template<typename key,typename value>
bool rb_tree<key,value>::Delete(const key& _key,value _value){
     std::vector<value> container;
     bool result=false;
     result=Search(_key,container);
     if(result==false)
        return false;
     for(int i=0;i<container.size();i++){
        if(container[i]==_value){
           result=true;
           break;
        }          
     }
     if(result==false)
        return false;
     node<key,value>* prenode=m_root;
     node<key,value>* nextnode=nullptr;
     node<key,value>* parentnode=nullptr;
     while(prenode!=m_sentiel){
       if(prenode->search<_key)
          prenode=prenode->next[2];
       else if(prenode->search<key)
          prenode=prenode->next[1];   
       else if(prenode->search==_key && prenode->data!=_value)
          prenode=prenode->next[1];
       else{
           prenode=delete_node(prenode);
           break;         
        }    
     }
     return true;   
} 

template<typename key,typename value>
void rb_tree<key,value>::Adjust(node<key,value>*& position){
     if(!position || position==m_sentiel || position->color==black)
        return;
     node<key,value>* prenode=position;
     while(prenode!=m_root && (prenode->next[0])->color==red){
         node<key,value>*  parentnode=prenode->next[0];
         node<key,value>* grandparentnode=parentnode->next[0];
         node<key,value>*  unclenode;
         if(parentnode==grandparentnode->next[1]){
            unclenode=grandparentnode->next[2];
           if(unclenode->color==red){
              unclenode->color=black;
              parentnode->color=black;
              grandparentnode->color=red;
              prenode=grandparentnode;
           }
           else if(unclenode->color==black){
              if(prenode==parentnode->next[2]){
               prenode=prenode->next[0];
               left_roatate(prenode);    
              }
              prenode->next[0]->color=black;
              prenode->next[0]->next[0]->color=red;
              right_roatate(prenode->next[0]->next[0]);
           }
         }
         else if(parentnode==grandparentnode->next[2]){
            unclenode=grandparentnode->next[1];
            if(unclenode->color==red){
               unclenode->color=black;
               parentnode->color=black;
               grandparentnode->color=red;
               prenode=grandparentnode;
            }
            else if(unclenode->color==black){
               if(prenode==parentnode->next[1]){
                  prenode=prenode->next[0];
                  right_roatate(prenode);
               }
                prenode->next[0]->color=black;
                prenode->next[0]->next[0]->color=red;
                left_roatate(prenode->next[0]->next[0]);
            }      
         } 
     }   
}//当出现两个连续的红色节点时进行调整

//进行左旋和右旋操作时，不改变节点的颜色
template<typename key,typename value>
void rb_tree<key,value>::left_roatate(node<key,value>*& position){
     if(position==m_sentiel || position->next[2]==m_sentiel)
        return;
     //position的右子树节点不为哨兵节点   
     node<key,value>* parentnode=position->next[0];   
     node<key,value>* rightnode=position->next[2];
     rightnode->next[0]=parentnode;
     if(parentnode!=m_sentiel){
        if(position==parentnode->next[1])
            parentnode->next[1]=rightnode;
        else{
         parentnode->next[2]=rightnode;
        }            
     }
     position->next[2]=rightnode->next[1];
     if(rightnode->next[1]!=m_sentiel)
        rightnode->next[1]->next[0]=position;
     rightnode->next[1]=position;
     position->next[0]=rightnode;     
}

template<typename key,typename value>
void rb_tree<key,value>::right_roatate(node<key,value>*& position){
     if(position==m_sentiel || position->next[1]==m_sentiel)
        return;
     //position的左子树节点不为哨兵节点   
      node<key,value>* parentnode=position->next[0];  
      node<key,value>* leftnode=position->next[1];
      leftnode->next[0]=parentnode;
      if(parentnode!=m_sentiel){
         if(position==parentnode->next[1]){
            parentnode->next[1]=leftnode;
         }
         else{
             parentnode->next[2]=leftnode;
         }
      }
      position->next[0]=leftnode;
      position->next[1]=leftnode->next[2];
      if(leftnode->next[2]!=m_sentiel)
         leftnode->next[2]->next[0]=position;
      leftnode->next[2]=position;
}


template<typename key,typename value>
node<key,value>*& rb_tree<key,value>::delete_node(node<key,value>*& position){
   if(position==nullptr || position==m_sentiel){
      std::cout<<"##the node may be deleted or that is sential node##"<<std::endl;
      return position;
   }     
   node<key,value>* parentnode=position->next[0];
   node<key,value>* newposition;
   int left_height=height(position->next[1]);
   int right_height=height(position->next[2]);
   if(position->next[1]==m_sentiel && position->next[2]==m_sentiel){
      if(position->next[0]!=m_sentiel){
          if(position==position->next[0]->next[1]){
           position=delete_max(position->next[0]);
          }
          else if(position==position->next[0]->next[2]){
           position=delete_min(position->next[0]);
          }
      }
      delete(position);
      position=NULL;
      return m_sentiel;
   }//当删除节点不存在子节点时
   if(left_height>right_height){
     newposition=delete_max(position);
   }
   else{
     newposition=delete_min(position);
   }
   parentnode=position->next[0];
   newposition->color=position->color;
   newposition->next[0]=parentnode;
   newposition->next[1]=position->next[1];
   newposition->next[2]=position->next[2];
   if(parentnode!=m_sentiel){
      if(position==parentnode->next[1])
         parentnode->next[1]=newposition;
       else if(position==parentnode->next[2])
          parentnode->next[2]=newposition;  
   }
   delete(position);
   position=NULL; 
   return newposition;
}


template<typename key,typename value>
bool rb_tree<key,value>::Search(const key& _key,std::vector<value>& container){
   container.clear();
   bool result=false;
   node<key,value>* pre=m_root;
   while(pre!=m_sentiel){
      if(_key<pre->search)
         pre=pre->next[1];
      else if(_key>pre->search){
         pre=pre->next[2];
      }
      else if(_key==pre->search){
         result=true;
         container.push_back(pre->data);
      }         
   }
    return result; 
}

template<typename key,typename value>
void rb_tree<key,value>::Print_value(const key& _key){
   std::vector<value> container;
   bool result=Search(_key,container);
   if(result==false)
      return;
   std::cout<<"##the value list of"<<_key<<": ";   
   for(int i=0;i<container.size();i++){
      std::cout<<container[i];
      if(i<container.size()-1)
        std::cout<<", ";
   }
   std::cout<<"##"<<std::endl;   
}

template<typename key,typename value>
node<key,value>*& rb_tree<key,value>::delete_max(node<key,value>*& position){
    if(position==m_sentiel || position->next[1]==m_sentiel)
      return m_sentiel;
    node<key,value>* prenode=position->next[1];
    node<key,value>* parentnode=position;
    node<key,value>* currentnode;
    while (prenode->next[2]!=m_sentiel){
      parentnode=prenode;
      prenode=prenode->next[2];
    }
    currentnode=prenode;
    if(currentnode->color==black){
      if(currentnode->next[1]->color==red){
        currentnode->next[1]->color=black; 
        parentnode->next[2]=currentnode->next[1];
        currentnode->next[1]->next[0]=parentnode;
      }
      else if(currentnode->next[1]->color==black){
        node<key,value>* brothernode=parentnode->next[1];
        if(brothernode->color==black){
           if(brothernode->next[1]->color==black && brothernode->next[2]->color==black){
              brothernode->color=red;
              parentnode->next[2]=m_sentiel;
              if(parentnode->color==red)
                 Adjust(brothernode);
           }
           else{
              if(brothernode->next[2]->color==red)
                 left_roatate(brothernode);
               right_roatate(parentnode);
               parentnode->next[0]->color=parentnode->color;
               parentnode->color=black;
               parentnode->next[2]=m_sentiel;  
           }
        }
        else if(brothernode->color==red){
            left_roatate(brothernode);
            right_roatate(parentnode);
            parentnode->next[2]=m_sentiel;
        }        
      }
    }
    else if(currentnode->color==red){
       parentnode->next[2]=currentnode->next[1];
       if(currentnode->next[1]->color!=m_sentiel)
         currentnode->next[1]->next[0]=parentnode;
    }
    return currentnode;  
}

template<typename key,typename value>
node<key,value>*& rb_tree<key,value>::delete_min(node<key,value>*& position){
   if(position==m_sentiel || position->next[2]==m_sentiel)
      return m_sentiel;
    node<key,value>* prenode=position->next[2];
    node<key,value>* currentnode=prenode;
    node<key,value>* parentnode=position;
    while(prenode->next[1]!=m_sentiel){
       parentnode=prenode;
       prenode=prenode->next[1];
    }
    currentnode=prenode;
    if(currentnode->color==black){
       if(currentnode->next[2]->color==red){
         currentnode->next[2]->color=black;
         parentnode->next[1]=currentnode->next[2];
         currentnode->next[2]->next[0]=parentnode;
       }
       else if(currentnode->next[2]->color==black){
          node<key,value>* brothernode=parentnode->next[2];
          if(brothernode->color==black){
            if(brothernode->next[1]->color==black && brothernode->next[2]->color==black){
               brothernode->color=red;
               parentnode->next[1]=m_sentiel;
               if(parentnode->color==red)
                 Adjust(brothernode);
            }
            else{   
               if(brothernode->next[1]->color==red)
                  right_roatate(brothernode);
               left_roatate(parentnode);
               parentnode->next[0]->color=parentnode->color;
               parentnode->next[0]->next[2]->color=black;
               parentnode->color=black;
               parentnode->next[1]=m_sentiel;
               parentnode->next[2]=m_sentiel;
            }
         }//兄弟节点为黑色
          else if(brothernode->color==red){
            right_roatate(brothernode);
            left_roatate(parentnode);
            parentnode->next[1]=m_sentiel;
            parentnode->next[2]=m_sentiel;
         }          
       }
    }
    else if(currentnode->color==red){
      parentnode->next[1]=currentnode->next[2];
      if(currentnode->next[2]!=m_sentiel)
         currentnode->next[2]->next[0]=parentnode; 
    }
    return currentnode;
}

template<typename key,typename value>
int rb_tree<key,value>::height(node<key,value>*& position){
   if(position==nullptr || position==m_sentiel)
     return 0;  
   return (height(position->next[1])>height(position->next[2])?height(position->next[1]):height(position->next[2]))+1;  
}

template<typename key,typename value>
int rb_tree<key,value>::height(){
   return height(m_root);
}

#endif
