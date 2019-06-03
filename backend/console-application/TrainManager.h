//
// Created by jinho on 3/12/2019.
//
#pragma once
#ifndef TTRS_BACKEND_TRAIN_H
#define TTRS_BACKEND_TRAIN_H

//todo:看看存储顺序能不能改进，更加符合连续性原理，更加cache-friendly


#include <table/table_heap.h>
#include "../util.h"
#include "TicketQuery.h"


namespace sjtu {

    class TicketQuery;

    struct train {
//        Trainid id;
        Name name;
        Catalog catalog;
        char locnum;
        char ticketkindnum;
        RID loctimepricePos;
        //address_t pricesPos;
        RID ticketleftPos1;
        RID ticketleftPos2;
        RID ticketleftPos3;
        TicketKind ticketKind[5];
    };

//    template <int locnum>
//    struct LocAndTime{
//        Loc stations[locnum];
//        Time timeArrive[locnum];
//        Time timeStart[locnum];
//        Time timeStopover[locnum];
//    };//没有空隙，OK(全部1byte）
//    template <int locnum,int ticketkindnum>
//    struct Prices{
//
//        TicketPrice  ticketPrice[locnum][ticketkindnum]{};//4byte
//
//    };//28byte
//    template <int ticketkindnum,int locnum,int datenum=30>
//    struct Ticketleft{
//        short ticket[datenum][ticketkindnum][locnum-1];
//    };
    struct stringcompare : public Comparator<Trainid> {
        bool operator() (const Trainid &a , const Trainid &b) const {
            return a.compareTo(b) < 0;
        }
    };

    class TrainManager {
        friend class Console;

        friend class TicketQuery;

    private:
        Bptree<Trainid , RID , stringcompare> trainStorage;
    public:
        TicketQuery *ticketQuery;
    private:
        TableHeap *trainTable = nullptr;
        TableHeap* loctimepriceTable= nullptr;

        TableHeap* ticketleftTable1= nullptr;
        TableHeap* ticketleftTable2= nullptr;
        TableHeap* ticketleftTable3= nullptr;
//        std::fstream loctimeprice;
//        std::fstream ticketleft;
//        MemoryController loctimepricemc;
//        MemoryController ticketleftmc;
        short defaultticketarray[9000];
    public:
        friend class TicketManager;

        TrainManager (sjtu::TicketQuery *ticketQuery= nullptr);

        ~TrainManager ();

        void printtrain (const sjtu::Trainid &trainid , std::ostream &os);

        inline train queryTrain (const Trainid &trainid , bool &success , Transaction *txn) {
            auto trainrid=trainStorage.get(trainid,success);
            if (!success) {
                return train{};
            }
            auto trainPage= reinterpret_cast<TablePage*>(recordbpm.FetchPage(trainrid.GetPageId()));
            Tuple thistrain;
            trainPage->GetTuple(trainrid,thistrain,txn);
            recordbpm.UnpinPage(trainrid.GetPageId(),false);
            train result;
            thistrain.SerializeTo(reinterpret_cast<char*>(&result));
            return result;
        }
        inline bool queryTrain (const Trainid& trainid,Tuple& tuple,RID& rid,Transaction* txn){
            bool success;
            rid=trainStorage.get(trainid,success);
            if (!success) {
                return false;
            }
            auto trainPage= reinterpret_cast<TablePage*>(recordbpm.FetchPage(rid.GetPageId()));
            recordbpm.UnpinPage(rid.GetPageId(),false);
            return trainPage->GetTuple(rid,tuple,txn);
        }

        bool addTrain (const Trainid &trainid , const Name &name , const Catalog &catalog , char locnum ,
                       char ticketkindnum ,
                       TicketKind *ticketKind , Loc *loc , Time *timearrive , Time *timestart , Time *timestopover ,TicketPrice *price);

        bool saletrain (const Trainid &trainid);

        bool deletetrain (const Trainid &trainid);

        bool modifytrain (const Trainid &trainid , const Name &name , const Catalog &catalog , char locnum ,char ticketkindnum ,TicketKind *ticketKind , Loc *loc , Time *timearrive , Time *timestart , Time *timestopover ,TicketPrice *price);

    };


}

#endif //TTRS_BACKEND_TRAIN_H
