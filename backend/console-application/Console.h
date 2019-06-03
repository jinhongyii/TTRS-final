//
// Created by jinho on 3/11/2019.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
#ifndef TTRS_BACKEND_CONSOLE_H
#define TTRS_BACKEND_CONSOLE_H

#include <iostream>
#include <table/table_heap.h>
#include "../util.h"
#include "TicketManager.h"
#include "UserManager.hpp"
#include "TrainManager.h"
#include "TicketQuery.h"

namespace sjtu {
    class Console {
        UserManager usermanager;
        TicketQuery  ticketQuery;
        TrainManager trainManager;
        TicketManager ticketManager;
    public:
        Console(){
            ticketQuery.trainManager=&trainManager;
            trainManager.ticketQuery=&ticketQuery;
            ticketManager.trainManager=&trainManager;
        }
        bool processline(std::istream& is,std::ostream& os){
            string<20> tmp;
            is>>tmp;
            if (tmp == string<10>("register")) {
                string<40>name;
                string<20>password;
                string<20> email;
                string<20> phone;
                is>>name>>password>>email>>phone;
                os<<usermanager.registerUser(name,password,email,phone)<<"\n";
                return false;
            } else if(tmp==string<6>("login")) {
                unsigned  long long userid;
                string<20> password;
                is>>userid>>password;
                os<<usermanager.login(userid,password)<<"\n";
                return false;
            } else if(tmp==string<15>("query_profile")) {
                unsigned long long userid;
                is>>userid;
                auto  user1_tuple= usermanager.search(userid , nullptr);
                user user1;
                user1_tuple.SerializeTo(reinterpret_cast<char*>(&user1));
                if (!user1_tuple.IsAllocated()) {
                    os<<0<<"\n";
                    return false;
                }
                os<<user1<<"\n";
                return false;
            } else if (tmp == string<15>("modify_profile")) {
                unsigned long long userid;
                string<40> name;
                string<20> password,email,phone;
                is>>userid>>name>>password>>email>>phone;
                os<<usermanager.modify(userid,name,password,email,phone)<<"\n";
                return false;
            } else if (tmp == string<20>("modify_privilege")) {
                unsigned long long id1,id2;
                int privilegde;
                is>>id1>>id2>>privilegde;
                os<<usermanager.changePriviledge(id1,id2,privilegde)<<"\n";
                return false;
            } else if (tmp == string<5>("exit")) {
                os<<"BYE"<<"\n";
                return true;
            } else if(tmp==string<10>("add_train")){
                Trainid trainid;
                Name name;
                Catalog catalog;
                int locnum,ticketkindnum;
                TicketKind ticketKind[5];
                Loc loc[60];
                Time timearrive[60];
                Time timestart[60];
                Time timestopover[60];
                TicketPrice price[300];
                is>>trainid>>name>>catalog>>locnum>>ticketkindnum;
                for (int i = 0; i < ticketkindnum; i++) {
                    is>>ticketKind[i];
                }
                int tmp=0;
                for (int i = 0; i < locnum; i++) {
                    is>>loc[i]>>timearrive[i]>>timestart[i]>>timestopover[i];
                    for (int j = 0; j < ticketkindnum; j++) {
                        char sign;
                        is>>sign;
                        is>>sign;
                        is>>sign;
                        is>>price[tmp+j];
                    }
                    tmp+=ticketkindnum;
                }
                os<<trainManager.addTrain(trainid,name,catalog,locnum,ticketkindnum,ticketKind,loc,timearrive,
                        timestart,timestopover,price)<<"\n";
                return false;
            } else if(tmp==string<11>("query_train")){
                Trainid trainid;
                is>>trainid;
                trainManager.printtrain(trainid , os);
                return false;
            } else if(tmp==string<10>("sale_train")) {
                Trainid trainid;
                is>>trainid;
                os<<trainManager.saletrain(trainid)<<"\n";
                return false;
            } else if (tmp == string<12>("delete_train")) {
                Trainid trainid;
                is>>trainid;
                os<<trainManager.deletetrain(trainid)<<"\n";
                return false;
            } else if(tmp==string<12>("modify_train")) {
                Trainid trainid;
                Name name;
                Catalog catalog;
                int locnum,ticketkindnum;
                TicketKind ticketKind[5];
                Loc loc[60];
                Time timearrive[60];
                Time timestart[60];
                Time timestopover[60];
                TicketPrice price[300];
                is>>trainid>>name>>catalog>>locnum>>ticketkindnum;
                for (int i = 0; i < ticketkindnum; i++) {
                    is>>ticketKind[i];
                }
                int tmp=0;
                for (int i = 0; i < locnum; i++) {
                    is>>loc[i]>>timearrive[i]>>timestart[i]>>timestopover[i];
                    for (int j = 0; j < ticketkindnum; j++) {
                        char sign;
                        is>>sign;
                        is>>sign;
                        is>>sign;
                        is>>price[tmp+j];
                    }
                    tmp+=ticketkindnum;
                }
                os<<trainManager.modifytrain(trainid,name,catalog,locnum,ticketkindnum,ticketKind,loc,timearrive,
                                            timestart,timestopover,price)<<"\n";
                return false;
            } else if(tmp==string<5>("clean")) {
                recordbpm.clear();
//                recordbpm.clear();
//                recorddisk.clear() ;
                recorddisk.clear();
                transactionManager.set_next_txn_id(0);
                logManager.set_next_lsn(0);

                usermanager.~UserManager();
                trainManager.~TrainManager();
                ticketQuery.~TicketQuery();
                ticketManager.~TicketManager();
                transactionManager.~TransactionManager();

                logManager.~LogManager();
                recorddisk.~DiskManager();
                new(&recorddisk)DiskManager("record.db");
                new (&logManager) LogManager(&recorddisk);
                new (&transactionManager) TransactionManager(&logManager,&recordbpm);

                new (&usermanager) UserManager();
                new (&trainManager)TrainManager();
                new (&ticketQuery) TicketQuery();
                new (&ticketManager) TicketManager();
                ticketQuery.trainManager=&trainManager;
                trainManager.ticketQuery=&ticketQuery;
                ticketManager.trainManager=&trainManager;
                os<<1<<"\n";
                return false;
            } else if(tmp==string<12>("query_ticket")) {

                Loc loc1,loc2;
                Date date;
                CatalogList catalogList;
                is>>loc1>>loc2>>date>>catalogList;
                vector<pair<Trainid,ticketinfo>> trains(100);
                ticketQuery.queryTicket(loc1 , loc2 , datetonum(date) , catalogList , trains);

                os<<trains.size()<<"\n";
                queryresult tmp;
                for (int i = 0; i < trains.size(); i++) {
                    ticketQuery.process(trains[i].second , trains[i].first , loc1 , loc2 , datetonum
                            (date),tmp);
                    os<<tmp.trainid<<" "<<tmp.loc1<<" "<<numtodate(tmp.date1)<<" "<<tmp.time1<<" "<<tmp
                    .loc2<<" "<<numtodate(tmp.date2)<<" "<<tmp.time2<<" ";
                    for (int j = 0; j<tmp.ticketkindnum; j++) {
                        os<<tmp.ticketKind[j]<<" "<<tmp.realticketleft[j]<<" "<<tmp.ticketPrice[j]<<" ";
                    }
                    os<<"\n";
                }
                return false;
            } else if (tmp == string<10>("buy_ticket")) {
                Userid userid;
                int num;
                Trainid trainid;
                Loc loc1,loc2;
                Date date;
                TicketKind ticketKind;
                is>>userid>>num>>trainid>>loc1>>loc2>>date>>ticketKind;
                os<<ticketManager.buyTicket(userid,num,trainid,loc1,loc2,datetonum(date),ticketKind)<<"\n";
            } else if(tmp==string<11>("query_order")) {
                Userid userid;
                Date date;
                CatalogList catalogList;
                is>>userid>>date>>catalogList;
                ticketManager.queryOrder(userid,datetonum(date),catalogList,os);
            } else if (tmp == string<13>("refund_ticket")) {
                Userid userid;
                int refundnum;
                Trainid trainid;
                Loc loc1,loc2;
                Date date;
                TicketKind ticketKind;
                is>>userid>>refundnum>>trainid>>loc1>>loc2>>date>>ticketKind;
                os<<ticketManager.refundTicket(userid,refundnum,trainid,loc1,loc2,datetonum(date),ticketKind)<<"\n";
            } else if(tmp==string<14>("query_transfer")) {
                Loc loc1,loc2,transloc;
                Date date;
                CatalogList catalogList;
                is>>loc1>>loc2>>date>>catalogList;
                auto trains= ticketQuery.queryTransfer(loc1 , loc2 , datetonum(date) , catalogList , transloc);
                if (trains.empty()) {
                    os<<-1<<"\n";
                } else {

                    auto tmp1=ticketQuery.process(trains[0].second,trains[0].first,loc1,transloc,datetonum(date));
                    os<<tmp1.trainid<<" "<<tmp1.loc1<<" "<<numtodate(tmp1.date1)<<" "<<tmp1.time1<<" "<<tmp1
                            .loc2<<" "<<numtodate(tmp1.date2)<<" "<<tmp1.time2<<" ";
                    for (int j = 0; !(tmp1.ticketKind[j] == string<5>("")); j++) {
                        os<<tmp1.ticketKind[j]<<" "<<tmp1.realticketleft[j]<<" "<<tmp1.ticketPrice[j]<<" ";
                    }
                    os<<"\n";
                    auto tmp2=ticketQuery.process(trains[1].second,trains[1].first,transloc,loc2,datetonum(date));
                    os<<tmp2.trainid<<" "<<tmp2.loc1<<" "<<numtodate(tmp2.date1)<<" "<<tmp2.time1<<" "<<tmp2
                            .loc2<<" "<<numtodate(tmp2.date2)<<" "<<tmp2.time2<<" ";
                    for (int j = 0; !(tmp2.ticketKind[j] == string<5>("")); j++) {
                        os<<tmp2.ticketKind[j]<<" "<<tmp2.realticketleft[j]<<" "<<tmp2.ticketPrice[j]<<" ";
                    }
                    os<<"\n";

                }
            } else if (tmp == string<20>("query_all_order")) {
                Userid  userid;
                is>>userid;
                ticketManager.queryAllOrder(userid,os);
            }
            return false;
        }
    };

}

#endif //TTRS_BACKEND_CONSOLE_H

#pragma clang diagnostic pop