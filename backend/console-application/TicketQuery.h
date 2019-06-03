//
// Created by jinho on 4/5/2019.
//
#ifndef TTRS_1_TICKETQUERY_H
#define TTRS_1_TICKETQUERY_H

#include "../util.h"
//#include "TrainManager.h"

namespace sjtu{
    class TrainManager;
    struct train;

struct trkey {
    Loc loc;
    Trainid trainid;
};
struct trval{
    char locno;
    Catalog catalog;
    char datefix1;
    char datefix2;
    Time timearrive;
    Time timestart;
};
struct ticketinfo{
    char locno1,locno2,datefix1,datefix2;
    Time timestart;
    Time timearrive;
};
struct queryresult {
    Trainid trainid;
    Loc loc1,loc2;
    int date1{},date2{};
    Time time1,time2;
    TicketKind ticketKind[5];
    int realticketleft[5]{};
    TicketPrice ticketPrice[5]{};
    int ticketkindnum{};
};
struct compare:public Comparator<trkey>{
    bool operator()( const trkey& k1, const trkey& k2)const{
        int cmp=k1.loc.compareTo(k2.loc);
        if (cmp<0) {
            return true;
        } else if(cmp>0) {
            return false;
        } else {
            cmp=k1.trainid.compareTo(k2.trainid);
            return cmp<0;
        }
    }
};

class TicketQuery {
    friend class TrainManager;
    Bptree<trkey,trval,compare> ticketq;
public:
    TrainManager * trainManager;
public:
    TicketQuery( TrainManager* trainManager= nullptr):ticketq("query"),trainManager
    (trainManager){}
    void queryTicket (const Loc &loc1 , const Loc &loc2 , const int &date , const
    CatalogList &catalogl , vector <pair<Trainid , ticketinfo>> &answer);
    void process (const ticketinfo &info , const Trainid &trainid , const Loc &loc1 , const Loc &loc2 ,
                         int date,queryresult& answer);
    vector <pair<Trainid ,train>> queryTransfer (const Loc &loc1 , const Loc &loc2 , const int &date , const
    CatalogList &catalogl , Loc &transloc);


    queryresult process (const train &tr , const Trainid &trainid , const Loc &loc1 , const Loc &loc2 , int date);
};

}

#endif //TTRS_1_TICKETQUERY_H
