//
// Created by jinho on 3/6/2019.
//
#pragma once
#ifndef TTRS_BACKEND_MEMORY_H
#define TTRS_BACKEND_MEMORY_H

#include <fstream>
#include "exceptions.hpp"
#include "string.hpp"
typedef int address_t;
namespace sjtu {
    class MemoryController {
        struct node {
            address_t start , end;
            node *prev , *next;

            node (long long start = 0 , long long end = 0 , node *prev = nullptr , node *next = nullptr) : start(
                    start) , end(end) , prev(prev) ,
                                                                                                           next
                                                                                                                   (next) {}
        };

        node *head , *tail;
        int nodecnt = 0;

//        node *toArray ();

    public:
        MemoryController ( address_t capacity = 0x7fffffff);

        MemoryController (const sjtu::string<20>& filename);

        MemoryController (const MemoryController &other) {
            auto p = head = new node;
            for (auto i = other.head->next; i != other.tail; i = i->next) {
                p->next = new node(i->start , i->end , p , nullptr);
                p = p->next;
            }
            tail = p->next = new node(0 , 0 , p , nullptr);
            nodecnt = other.nodecnt;
        }

        MemoryController &operator= (const MemoryController &other) {
            this->~MemoryController();
            new(this) MemoryController(other);
            return *this;
        }

        address_t malloc (int size);

        void free (address_t start , int size);

        void savecontroller (const sjtu::string<20>&);

        ~MemoryController ();

    };
}

#endif //TTRS_BACKEND_MEMORY_H
