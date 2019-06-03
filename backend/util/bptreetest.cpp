#include <iostream>
#include <climits>
#include "bptree.hpp"

using namespace sjtu;
using std::cout;using std::endl;

struct intnum {
    int a;
    explicit intnum(int a=0):a(a){}

};
std::ostream&operator<<(std::ostream& os,const intnum& a){
    os<<a.a;
    return os;
}

struct compare {
    bool operator()( intnum a1, intnum a2)const{
        return a1.a<a2.a;
    }
};
int main () {
    LogRecovery logRecovery(&recorddisk,&recordbpm,&logManager,&transactionManager);
    int num[]={2,15,8,14,9,1,3,5,4,11,13,12,10,7,6};
    int num2[]={4,8,10,3,12,11,7,1,9,5,6,2,15,14,13};
    recorddisk.clear();
    auto* a= new Bptree<intnum , RID , compare>("happy");
    for (int i = 0; i < 15; i ++) {
        Transaction* txn=transactionManager.Begin();
        a->put(intnum(num[i]) , RID(num[i],num[i]) , txn);
        transactionManager.Commit(txn);
        a->completePrint();
//        cout<<a->containsKey(intnum(num[i]))<<endl;
    }
    delete a;
    recordbpm.clear();

    logRecovery.Redo();
    logRecovery.Undo();
    a= new Bptree<intnum , RID , compare>("happy");
    for (int i = 0; i < 15; i++) {
        bool success;
        cout<<a->get(intnum(i),success)<<' ';
    }
    delete a;
//    for (int i = 14; i >=4; i--) {
//        std::cout<<"i="<<i<<std::endl;
//        a->completePrint();
//        Transaction* txn=transactionManager.Begin();
//        a->remove(intnum(num2[i]) , txn);
//        transactionManager.Commit(txn);
//        std::cout<<"erase "<<num2[i]<<std::endl;
//        cout<<a->containsKey(intnum(num2[i]))<<endl;
//        std::cout<<"\n\n";
//    }
//
//
//    cout<<a<<endl;
//    delete a;
//    auto *b= new Bptree<intnum , intnum , compare>("happy");
////
//    cout<<b<<endl;
//    for (int i = 0; i < 10; i ++) {
//        cout<<"put "<<num[i]<<endl;
//        Transaction* txn=transactionManager.Begin();
//        b->put(intnum(num[i]) , intnum(num[i]) , txn);
//        transactionManager.Commit(txn);
//        b->completePrint();
//    }
//
//
//    for (int i = 0; i < 15; i ++) {
//        cout<<"put "<<num[i]<<endl;
//        Transaction* txn=transactionManager.Begin();
//        b->put(intnum(num[i]) , intnum(num[i]) , txn);
//        transactionManager.Commit(txn);
//        b->completePrint();
//
//    }
//
//    for (int i = 0; i < 15; i++) {
//        Transaction* txn=transactionManager.Begin();
//        b->put(intnum(num2[i]) , intnum(num2[i]) , txn);
//        transactionManager.Commit(txn);
//    }
//    auto i= b->rangeSearch(intnum(-1) , intnum(13) , <#initializer#>);
//    for (int j = 0; j < i.size(); j++) {
//        cout<<i[j].second<<" ";
//    }

//    try {
//        b->rangeSearch(intnum(16) , intnum(100) , <#initializer#>);
//    } catch (container_is_empty) {
//        cnt++;
//    }
//
//    delete b;


    return 0;
//    std::ios::sync_with_stdio(false);
//    cout.tie(0);
//    int t=clock();
//    Bptree<int,int> aa;
//    aa.clear();
//    for (int i = 0; i < 1e5; i++) {
//        aa.put(i,i);
//    }
//    for (int i = 0; i < 1e5; i++) {
//        cout<<aa.get(i)<<" ";
//    }
//    cout<<endl;
//    for (int i = 0; i < 1e5; i++) {
//        cout<<aa.containsKey(i);
//    }
//    cout<<endl;
//    for (int i = 0; i < 1e5; i++) {
//        aa.remove(i);
//    }
//    cout<<(double)(clock()-t)/CLOCKS_PER_SEC;
//cout<<sizeof(Bptree<int,int >::InnerNode);
}