//
// Created by jinho on 4/30/2019.
//

#ifndef TTRS_2_LIST_H
#define TTRS_2_LIST_H

#include <cstdlib>
namespace  sjtu {
    template <class T>
    class list {
        struct node{
            node* next;
            node* prev;
            T data;
            node(T data,node* next= nullptr,node* prev=nullptr):data(data),next(next),prev(prev){}
        };
        node* head;
        node* tail;
        int sz;
    public:
        struct iterator{
            node* ptr;
            iterator(node* ptr= nullptr):ptr(ptr){}
            iterator& operator++(){
                ptr=ptr->next;
            }
            T& operator*(){
                return ptr->data;
            }
            bool operator!=(const iterator& other){
                return other.ptr!=ptr;
            }
            iterator&operator--(){
                ptr=ptr->prev;
            }
        };


        list(){
            head=(node*)(malloc(sizeof(node)));
            tail=(node*)(malloc(sizeof(node)));
            head->next=tail;
            tail->prev=head;
            sz=0;
        }
        ~list (){
            for (auto p = head->next; p != tail;) {
                auto tmp=p;
                p=p->next;
                delete tmp;
            }
            free(head);
            free(tail);
        }
        inline void pop_front(){
            auto tmp=head->next;
            head->next=head->next->next;
            head->next->prev=head;
            delete tmp;
            sz--;
        }
        inline void pop_back(){
            auto tmp=tail->prev;
            tail->prev=tail->prev->prev;
            tail->prev->next=tail;
            delete tmp;
            sz--;
        }
        inline void push_back(const T& data){
            tail->prev=new node(data,tail,tail->prev);
            tail->prev->prev->next=tail->prev;
            sz++;
        }
        inline void push_front(const T& data){
            head->next=new node(data,head->next,head);
            head->next->next->prev=head->next;
            sz++;
        }
        inline T& back(){
            return tail->prev->data;
        }
        inline T& front(){
            return head->next->data;
        }
        inline int size(){ return sz;}

        inline iterator begin(){
            return iterator(head->next);
        }
        inline iterator end(){
            return iterator(tail);
        }
        inline void erase( iterator it){
            it.ptr->prev->next=it.ptr->next;
            it.ptr->next->prev=it.ptr->prev;
            delete it.ptr;
        }
        inline void clear(){
            for (auto p = head->next; p != tail;) {
                auto tmp =p;
                p=p->next;
                delete tmp;
            }
            head->next=tail;
            tail->prev=head;
        }
        inline bool empty(){ return size()==0;}

    };
}
#endif //TTRS_2_LIST_H
