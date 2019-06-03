//
// Created by jinho on 3/11/2019.
//

#include "Console.h"
using namespace sjtu;
int main(int argc,char* argv[]){
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);
    bool flag=true;
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i] , "--nologging")==0) {
        ENABLE_LOGGING=false;
      } else if (strcmp(argv[i] , "--norecovery")==0) {
        flag=false;
      }
    }
    if (flag) {
      logRecovery.Redo();
      logRecovery.Undo();
    }
    //    std::fstream f("../backend/test_kit/8/1.in");
    Console console;
    while (true) {
        if(console.processline(std::cin,std::cout)) {
            break;
        }
        std::cout.flush();
//        auto page=recordbpm.FetchPage(118);
//        std::cerr<<(page?page->GetPageId():-1)<<std::endl;
//        recordbpm.UnpinPage(118,false);
    }


}