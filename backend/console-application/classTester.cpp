//
// Created by jinho on 4/5/2019.
//
#include "Console.h"
#include <iostream>
using namespace sjtu;
int main(){
    char* log=new char[1000000]{0};
    auto read_count=recorddisk.ReadLog(log,1000000,0);
    int has_read=0;
    while (has_read < read_count) {
        LogRecord* logRecord= reinterpret_cast<LogRecord*>(log+has_read);
        has_read+=logRecord->GetSize();
        std::cout<<logRecord->ToString()<<std::endl;
    }

}
