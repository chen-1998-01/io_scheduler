#ifndef _KEY_VALUE_H_
#define _KEY_VALUE_H_

#include<iostream>
#include<cmath>
#include<algorithm>

template<typename key,typename value>
struct node{
    key search;
    value data;
    int num;
    int balance_degree;
    int color;
    node<key,value>** next;
    node(const key& _search,const value& _data,const int& _num,const int& _color=0):
      search(_search),data(_data),next(nullptr),num(_num),balance_degree(0),color(_color){
            next=new node<key,value> *[num];
            for(int i=0;i<num;i++)
                next[i]=NULL;
    };
    node(const node& obj){
        search=obj.search;
        data=obj.data;
        next=obj.next;
        num=obj.num;
        balance_degree=obj.balance_degree;
        next=new node<key,value>*[num];
        for(int i=0;i<num;i++){
            next[i]=obj->next[i];
        }
    }
    ~node(){
        if(next){
            for(int i=0;i<num;i++){
                delete(next[i]);
                next[i]=nullptr;
            }
            delete[] next;
        }
    }
    friend bool operator<(const node<key,value>& obj,const node<key,value>& obj2){
        return obj.search<obj2.search;
    }

    std::ostream& operator<<(std::ostream& out,node<key,value>& obj){
        out<<obj.search<<","<<obj.data<<" ";
        return out;
    }

}; 




#endif