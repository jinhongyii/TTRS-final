//
// Created by jinho on 3/12/2019.
//

#include "TrainManager.h"
#include "TicketQuery.h"
bool sjtu::TrainManager::saletrain (const sjtu::Trainid &trainid) {
    Transaction* txn=transactionManager.Begin();
    Tuple thistrain;
    RID train_rid;
    bool success=queryTrain(trainid,thistrain,train_rid,txn);
    train& tr=*(reinterpret_cast<train*>(thistrain.GetData()));
//    thistrain.SerializeTo(reinterpret_cast<char*>(&tr));
    if (!success|| tr.ticketleftPos1.GetPageId()!=INVALID_PAGE_ID) {
        transactionManager.Abort(txn);
        delete txn;
        return false;
    }
    Tuple ticketleft;
    ticketleft.DeserializeFrom(reinterpret_cast<char*>(defaultticketarray),sizeof(short)*(tr.locnum-1)*tr
    .ticketkindnum*(datenum/3));

    RID rid1,rid2,rid3;
    ticketleftTable1->InsertTuple(ticketleft,rid1,txn);
    ticketleftTable2->InsertTuple(ticketleft,rid2,txn);
    ticketleftTable3->InsertTuple(ticketleft,rid3,txn);
    tr.ticketleftPos1=rid1;
    tr.ticketleftPos2=rid2;
    tr.ticketleftPos3=rid3;
//    thistrain.DeserializeFrom(reinterpret_cast<char*>(&tr),sizeof(tr));
    trainTable->UpdateTuple(thistrain,train_rid,txn);
    transactionManager.Commit(txn);
    delete txn;
    return true;
}



bool
sjtu::TrainManager::addTrain (const sjtu::Trainid &trainid , const sjtu::Name &name , const sjtu::Catalog &catalog ,
                              char locnum , char ticketkindnum , sjtu::TicketKind *ticketKind , sjtu::Loc *loc ,
                              sjtu::Time *timearrive , sjtu::Time *timestart , sjtu::Time *timestopover ,
                              sjtu::TicketPrice *price) {

    Transaction* txn=transactionManager.Begin();
    RID ltp_rid,train_rid;
    Tuple ltp,addedTrainTuple;
    char buf[PAGE_SIZE];
    int pos=0;
    TicketPrice tmpsum[5]{};
    for (int i = 0; i < locnum; i++) {
        for (int j = 0; j < ticketkindnum; j++) {
            tmpsum[j]+=price[i*ticketkindnum+j];
            price[i*ticketkindnum+j]=tmpsum[j];
        }
    }
    memcpy(buf,loc,sizeof(Loc)*locnum);
    pos+=sizeof(Loc)*locnum;
    memcpy(buf+pos,timearrive,sizeof(Time)*locnum);
    pos+=sizeof(Time)*locnum;
    memcpy(buf+pos,timestart,sizeof(Time)*locnum);
    pos+=sizeof(Time)*locnum;
    memcpy(buf+pos,timestopover,sizeof(Time)*locnum);
    pos+=sizeof(Time)*locnum;
    memcpy(buf+pos,price,sizeof(TicketPrice)*locnum*ticketkindnum);
    pos+=sizeof(TicketPrice)*locnum*ticketkindnum;


    ltp.DeserializeFrom(buf,pos);
    loctimepriceTable->InsertTuple(ltp,ltp_rid,txn);
    train addedtrain=train{name,catalog,locnum,ticketkindnum,ltp_rid};
    for (int i = 0; i < addedtrain.ticketkindnum; i++) {
        addedtrain.ticketKind[i]=ticketKind[i];
    }
    addedTrainTuple.DeserializeFrom(reinterpret_cast<char *>(&addedtrain) , sizeof(addedtrain));
    trainTable->InsertTuple(addedTrainTuple , train_rid,txn);
    trainStorage.put(trainid , train_rid , txn);//todo:may be wrong,but may be faster if bulk loading is implemented

    char datefix1=0,datefix2=0;
    for (int i = 0; i < locnum; i++) {
        if(i==1) {
            if (timearrive[i].compareTo(timestart[i - 1] )<0) {
                datefix1++;
            }
        } else if (i != 0 && timearrive[i].compareTo(timearrive[i - 1]) < 0) {
            datefix1++;
        }
        if (i != locnum - 1 && i != 0 && timestart[i].compareTo(timestart[i - 1])<0) {
            datefix2++;
        }
        ticketQuery->ticketq.put(trkey{loc[i],trainid},trval{(char)i,catalog,datefix1,datefix2,timearrive[i],
                                                             timestart[i]},txn);
    }



    transactionManager.Commit(txn);
    delete txn;
    return true;
}

void sjtu::TrainManager::printtrain (const sjtu::Trainid &trainid ,std::ostream& os) {

    bool success;
    auto i= queryTrain(trainid , success , nullptr);
    if (!success||i.ticketleftPos1.GetPageId()==INVALID_PAGE_ID) {
        os<<0<<std::endl;
        return;
    }
    os<< trainid << " " << i.name << " " << i.catalog << " " << (int) i.locnum << " " << (int) i.ticketkindnum
              << " ";
//    TicketKind ticketKinds[5];
//    ticketprices.seekg(i.pricesPos);
//    ticketprices.read(reinterpret_cast<char*>(ticketKinds),sizeof(TicketKind)*i.ticketkindnum);
    for (int j = 0; j < i.ticketkindnum; j++) {
        os<<i.ticketKind[j]<<" ";
    }
    os<<'\n';



    Tuple ltp;
    loctimepriceTable->GetTuple(i.loctimepricePos,ltp, nullptr);
    char*buf=ltp.GetData();
    Loc* stations= reinterpret_cast<Loc*>(buf);
    Time* timeArrive= reinterpret_cast<Time*>(buf+sizeof(Loc)*i.locnum);
    Time* timeStart= reinterpret_cast<Time*>(buf+(sizeof(Loc)+sizeof(Time))*i.locnum);
    Time* timeStopover= reinterpret_cast<Time*>(buf+(sizeof(Loc)+sizeof(Time)*2)*i.locnum);
    TicketPrice * ticketPrice= reinterpret_cast<TicketPrice *>(buf+(sizeof(Loc)+sizeof(Time)*3)*i.locnum);
    int tmp=0;
    for (int j = 0; j < i.locnum; j++) {
        os<<stations[j]<<" "<<timeArrive[j]<<" "<<timeStart[j]<<" "<<timeStopover[j]<<" ";
        for (int k = 0; k < i.ticketkindnum; k++) {
            os<<"￥"<<ticketPrice[tmp+k]-((tmp==0)?0:ticketPrice[tmp+k-i.ticketkindnum])<<" ";
        }
        os<<'\n';
        tmp+=i.ticketkindnum;
    }
}

sjtu::TrainManager::~TrainManager () {
    delete loctimepriceTable;
    delete trainTable;
    delete ticketleftTable1;
    delete ticketleftTable2;
    delete ticketleftTable3;
}

sjtu::TrainManager::TrainManager (sjtu::TicketQuery *ticketQuery) : ticketQuery(ticketQuery),
                                                                      trainStorage("trainsindex"){
    HeaderPage* head= reinterpret_cast<HeaderPage*>(recordbpm.FetchPage(0));
    page_id_t first_page;
    Transaction* txn_init=transactionManager.Begin();
    if(head->GetRootId("trains",first_page)){
        trainTable=new TableHeap(&recordbpm, &logManager,first_page);
    } else {
        trainTable = new TableHeap(&recordbpm , &logManager,txn_init);
        head->InsertRecord("trains",trainTable->GetFirstPageId(),txn_init,transactionManager.log_manager_);
    }
    if(head->GetRootId("loctimeprice",first_page)) {
        loctimepriceTable=new TableHeap(&recordbpm,&logManager,first_page);
    } else {
        loctimepriceTable=new TableHeap(&recordbpm,&logManager,txn_init);
        head->InsertRecord("loctimeprice",loctimepriceTable->GetFirstPageId(),txn_init,transactionManager.log_manager_);
    }

    if (head->GetRootId("ticketleft1" , first_page)) {
        ticketleftTable1=new TableHeap(&recordbpm,&logManager,first_page);
    } else{
        ticketleftTable1=new TableHeap(&recordbpm,&logManager,txn_init);
        head->InsertRecord("ticketleft1",ticketleftTable1->GetFirstPageId(),txn_init,transactionManager.log_manager_);
    }
    if (head->GetRootId("ticketleft2" , first_page)) {
        ticketleftTable2 = new TableHeap(&recordbpm , &logManager , first_page);
    } else {
        ticketleftTable2 = new TableHeap(&recordbpm , &logManager , txn_init);
        head->InsertRecord("ticketleft2",ticketleftTable2->GetFirstPageId(),txn_init,transactionManager.log_manager_);
    }
    if (head->GetRootId("ticketleft3",first_page)) {
        ticketleftTable3=new TableHeap(&recordbpm,&logManager,first_page);
    } else {
        ticketleftTable3=new TableHeap(&recordbpm,&logManager,txn_init);
        head->InsertRecord("ticketleft3",ticketleftTable3->GetFirstPageId(),txn_init,transactionManager.log_manager_);
    }
    recordbpm.UnpinPage(HEADER_PAGE_ID,true);
    transactionManager.Commit(txn_init);
    delete txn_init;
    for (int i = 0; i < 9000; i++) {
        defaultticketarray[i]=defaultTicket;
    }
}

bool sjtu::TrainManager::deletetrain (const sjtu::Trainid &trainid) {
    auto txn=transactionManager.Begin();
        try {
            auto deleted_train_rid= trainStorage.remove(trainid , txn);
            auto deletedTrainPage= reinterpret_cast<TablePage*>(recordbpm.FetchPage(deleted_train_rid.GetPageId()));
            Tuple deleted_tuple,deleted_loc;
            train deleted_train;
            char buf[3540];
            Loc* tmp= reinterpret_cast<Loc*>(buf);
            deletedTrainPage->GetTuple(deleted_train_rid,deleted_tuple,txn);
            recordbpm.UnpinPage(deleted_train_rid.GetPageId(),false);
            deleted_tuple.SerializeTo(reinterpret_cast<char*>(&deleted_train));
            if (deleted_train.ticketleftPos1.GetPageId() != INVALID_PAGE_ID) {
                transactionManager.Abort(txn);
                delete txn;
                return 0;
            }
            auto LocPage= reinterpret_cast<TablePage*>(recordbpm.FetchPage(deleted_train.loctimepricePos.GetPageId()));
            LocPage->GetTuple(deleted_train.loctimepricePos,deleted_loc,txn);
            recordbpm.UnpinPage(deleted_train.loctimepricePos.GetPageId(),false);
            deleted_loc.SerializeTo(buf);
            ticketleftTable1->MarkDelete(deleted_train.ticketleftPos1,txn);
            ticketleftTable2->MarkDelete(deleted_train.ticketleftPos2,txn);
            ticketleftTable3->MarkDelete(deleted_train.ticketleftPos3,txn);
            trainTable->MarkDelete(deleted_train_rid,txn);
            loctimepriceTable->MarkDelete(deleted_train.loctimepricePos,txn);

            for (int i = 0; i < deleted_train.locnum; i++) {
                ticketQuery->ticketq.remove(trkey{tmp[i] , trainid} , txn);
            }
            transactionManager.Commit(txn);
            delete txn;
            return 1;
        }
        catch (NoSuchElementException){
            transactionManager.Abort(txn);
            delete txn;
            return 0;
        }

}

bool
sjtu::TrainManager::modifytrain (const sjtu::Trainid &trainid , const sjtu::Name &name , const sjtu::Catalog &catalog ,
                                 char locnum , char ticketkindnum , sjtu::TicketKind *ticketKind , sjtu::Loc *loc ,
                                 sjtu::Time *timearrive , sjtu::Time *timestart , sjtu::Time *timestopover ,
                                 sjtu::TicketPrice *price) {
    return deletetrain(trainid) && addTrain(trainid,name,catalog,locnum,ticketkindnum,ticketKind,loc,timearrive,
            timestart,timestopover,price);
}





