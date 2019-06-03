//
// Created by jinho on 3/6/2019.
//


#include "MemoryController.h"

using namespace sjtu;
MemoryController::MemoryController (address_t capacity) {
    head=new node;
    tail=new node;
    head->next=new node(0,capacity-1,head,tail);
    nodecnt++;
    tail->prev=head->next;
}

address_t MemoryController::malloc (int size) {
    for (auto i = head->next; i != tail; i = i->next) {
        if (i->end - i->start+1 > size) {
            auto tmp=i->start;
            i->start+=size;
            return tmp;
        } else if(i->end - i->start+1 == size) {
            auto tmp=i->start;
            i->prev->next=i->next;
            i->next->prev=i->prev;
            delete i;
            nodecnt--;
            return tmp;
        }
    }
    return -1;
}

void MemoryController::free(address_t start,int size){
    auto p=head->next;
    for (; p != tail && p->start < start;p=p->next) {}
    if (p!=tail&&start + size == p->start) {
        p->start=start;
    } else {
        p=new node(start,start+size-1,p->prev,p);
        p->prev->next=p;
        p->next->prev=p;
        nodecnt++;
    }
    if (p->prev != head && p->prev->end == p->start - 1) {
        p->prev->end=p->end;
        p->prev->next=p->next;
        p->next->prev=p->prev;
        delete p;
        nodecnt--;
    }
}

//需要手动保存
void MemoryController::savecontroller (const sjtu::string<20>& memoryfilename) {
    std::fstream iofile(memoryfilename,std::fstream::out);
    iofile.write(reinterpret_cast<char*>(&nodecnt),sizeof(nodecnt));
//    node* tmp=toArray();
//    iofile.write(reinterpret_cast<char*>(tmp),sizeof(node)*nodecnt);
//    delete []tmp;
    address_t tmp[2];
    for (auto p = head->next; p != tail;p=p->next) {
        tmp[0]=p->start;
        tmp[1]=p->end;
        iofile.write(reinterpret_cast<char*>(tmp),sizeof(address_t)*2);
    }
    iofile.close();
}

//MemoryController::node* MemoryController::toArray () {
//    node* nodeArray=new node[nodecnt];
//    int cnt=0;
//    for (auto i = head->next; i != tail; i = i->next) {
//        nodeArray[cnt]=*i;
//        cnt++;
//    }
//    return nodeArray;
//}
MemoryController::MemoryController (const sjtu::string<20>& filename) {
    std::fstream iofile;
    iofile.open(filename);
    if (!iofile) {
        new(this) MemoryController();
    }
    else {
        iofile.read(reinterpret_cast<char *> (&nodecnt) , sizeof(nodecnt));
        node *nodeArray = new node[nodecnt];
        iofile.read(reinterpret_cast<char *> (nodeArray) , sizeof(node) * nodecnt);
        head = new node;
        auto p = head;
        for (int i = 0; i < nodecnt; i++) {
            p->next = new node(nodeArray[i].start , nodeArray[i].end , p , nullptr);
            p = p->next;
        }
        tail = new node;
        p->next = tail;
        tail->prev = p;
        delete[]nodeArray;
    }
}
MemoryController::~MemoryController () {
    for (auto i = head; i != nullptr;) {
        auto tmp=i;
        i=i->next;
        delete tmp;
    }

}
