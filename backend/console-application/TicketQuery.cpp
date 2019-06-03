#include "TicketQuery.h"
#include "TrainManager.h"
#include "TicketManager.h"

using namespace sjtu;
void
TicketQuery::queryTicket (const Loc &loc1 , const Loc &loc2 , const int &date , const
CatalogList &catalogl , vector<pair<Trainid , ticketinfo>> &answer) {
    const auto& tmp1=loc1;
    const auto& tmp2=loc2;

    vector<pair<trkey,trval>> trainarray1(100),trainarray2(100);
    struct comp:public Comparator<trkey>{
        bool operator()(const trkey& trkey1,const trkey& trkey2)const override{
            return trkey1.loc.compareTo(trkey2.loc)==0;
        }
    };
    comp c;
    ticketq.rangeSearch(trkey{loc1 , ""} , trainarray1,c);
    ticketq.rangeSearch(trkey{loc2 , ""}, trainarray2,c);
    auto it1=trainarray1.begin();
    auto it2=trainarray2.begin();
    while (it1 != trainarray1.end() && it2 != trainarray2.end()) {
        int cmp=(*it1).first.trainid.compareTo ((*it2).first.trainid);
        if (cmp<0) {
            it1++;
        } else if (cmp > 0) {
            it2++;
        } else {
            int sz=catalogl.size();
            for (int i=0;i<sz;i++) {
                if (catalogl[i] == (*it1).second.catalog) {
                    goto matchcatalog;
                }
            }
            it1++;
            it2++;
            continue;
            matchcatalog:
            if ((*it1).second.locno >= (*it2).second.locno) {
                it1++;
                it2++;
                continue;
            }
            //auto tr=trainManager->queryTrain((*it1).first.trainid);
            answer.push_back(pair<Trainid,ticketinfo>((*it1).first.trainid,ticketinfo{it1->second.locno,it2->second
            .locno,it1->second.datefix2,it2->second.datefix1,it1->second.timestart,it2->second.timearrive}));
            it1++;
            it2++;
        }
    }


}


void TicketQuery::process (const ticketinfo &info , const Trainid &trainid , const Loc &loc1 ,
                                  const Loc &loc2 ,
                                  int date,queryresult& answer) {
    bool success;
    auto tr= trainManager->queryTrain(trainid , success , nullptr);
    int j=info.locno2;
    int k = info.locno1;
    int date1=date+info.datefix1;
    int date2=date+info.datefix2;
    const Time& timearrive=info.timearrive,timestart=info.timestart;
    TicketPrice ticketPrices[10];
    Tuple ltp_tuple,tl_tuple;
    char ltp_buf[3540],tl_buf[6000];
    trainManager->loctimepriceTable->GetTuple(tr.loctimepricePos,ltp_tuple, nullptr);
    ltp_tuple.SerializeTo(ltp_buf);
    memcpy(ticketPrices,ltp_buf+(sizeof(Loc)+3*sizeof(Time))*tr.locnum+sizeof(TicketPrice)*tr
            .ticketkindnum*k,sizeof(TicketPrice)*tr.ticketkindnum);
    memcpy(ticketPrices+tr.ticketkindnum,ltp_buf+(sizeof(Loc)+3*sizeof(Time))*tr.locnum+sizeof(TicketPrice)*tr.ticketkindnum*j,sizeof(TicketPrice)*tr.ticketkindnum);
    short* ticketleft;
    if (date < datenum / 3) {
        trainManager->ticketleftTable1->GetTuple(tr.ticketleftPos1,tl_tuple, nullptr);
        tl_tuple.SerializeTo(tl_buf);
        ticketleft= reinterpret_cast<short*>(tl_buf+(date * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    } else if (date < 2 * datenum / 3) {
        trainManager->ticketleftTable2->GetTuple(tr.ticketleftPos2,tl_tuple, nullptr);
        tl_tuple.SerializeTo(tl_buf);
        ticketleft= reinterpret_cast<short*>(tl_buf+((date-datenum/3) * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    } else {
        trainManager->ticketleftTable3->GetTuple(tr.ticketleftPos3,tl_tuple, nullptr);
        tl_tuple.SerializeTo(tl_buf);
        ticketleft= reinterpret_cast<short*>(tl_buf+((date-2*datenum/3) * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    }

    short realticketleft[5]{defaultTicket , defaultTicket , defaultTicket , defaultTicket , defaultTicket};
    int tmp=0;
    for (int i = 0; i < tr.ticketkindnum; i++) {
        for (int t = k; t < j; t++) {
            realticketleft[i]=std::min(realticketleft[i],ticketleft[tmp+t]);
        }
        tmp+=tr.locnum-1;
    }

    answer.trainid=trainid;
    answer.loc1=loc1;
    answer.loc2=loc2;
    answer.time1=timestart;
    answer.time2=timearrive;
    answer.date1=date1;
    answer.date2=date2;
    answer.ticketkindnum=tr.ticketkindnum;
    for (int i = 0; i < tr.ticketkindnum; i++) {
        answer.ticketKind[i]=tr.ticketKind[i];
        answer.realticketleft[i]=realticketleft[i];
        answer.ticketPrice[i]=ticketPrices[i+tr.ticketkindnum]-ticketPrices[i];
    }


}
queryresult TicketQuery::process (const train &tr , const Trainid &trainid , const Loc &loc1 , const Loc &loc2 ,
                                   int date) {

    TicketPrice ticketPrices[10];
    int date1 = date , date2 = date;
    Tuple ltp_tuple,tl_tuple;
    char *ltp_buf,*tl_buf;
    trainManager->loctimepriceTable->GetTuple(tr.loctimepricePos,ltp_tuple, nullptr);
    ltp_buf=ltp_tuple.GetData();
    Loc* locs= reinterpret_cast<Loc*>(ltp_buf);
    Time* timearrive= reinterpret_cast<Time*>(ltp_buf+sizeof(Loc)*tr.locnum);
    Time* timestart= reinterpret_cast<Time*>(ltp_buf+(sizeof(Loc)+sizeof(Time))*tr.locnum);
    int j;
    for (j = 0; j < tr.locnum && locs[j] != loc1; j++) {
        if (j != 0 && timearrive[j].compareTo(timestart[j - 1]) < 0) {
            date1++;
            date2++;
        }
    }
    int k = j;
    for (; j < tr.locnum && locs[j] != loc2; j++) {
        if (j != 0 && timearrive[j].compareTo(timestart[j - 1]) < 0) {
            date2++;
        }
    }
    if (j == tr.locnum) {
        return queryresult{};
    }
    memcpy(ticketPrices,ltp_buf+(sizeof(Loc)+3*sizeof(Time))*tr.locnum+sizeof(TicketPrice)*tr
            .ticketkindnum*k,sizeof(TicketPrice)*tr.ticketkindnum);
    memcpy(ticketPrices+tr.ticketkindnum,ltp_buf+(sizeof(Loc)+3*sizeof(Time))*tr.locnum+sizeof(TicketPrice)*tr.ticketkindnum*j,sizeof(TicketPrice)*tr.ticketkindnum);
    short* ticketleft;
    if (date < datenum / 3) {
        trainManager->ticketleftTable1->GetTuple(tr.ticketleftPos1,tl_tuple, nullptr);
        tl_buf=tl_tuple.GetData();
        ticketleft= reinterpret_cast<short*>(tl_buf+(date * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    } else if (date < 2 * datenum / 3) {
        trainManager->ticketleftTable2->GetTuple(tr.ticketleftPos2,tl_tuple, nullptr);
        tl_buf=tl_tuple.GetData();
        ticketleft= reinterpret_cast<short*>(tl_buf+((date-datenum/3) * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    } else {
        trainManager->ticketleftTable3->GetTuple(tr.ticketleftPos3,tl_tuple, nullptr);
        tl_buf=tl_tuple.GetData();
        ticketleft= reinterpret_cast<short*>(tl_buf+((date-2*datenum/3) * tr.ticketkindnum * (tr.locnum - 1)) * sizeof(short));
    }
    short realticketleft[5]{defaultTicket , defaultTicket , defaultTicket , defaultTicket , defaultTicket};
    for (int i = 0; i < tr.ticketkindnum; i++) {
        for (int t = k; t < j; t++) {
            realticketleft[i]=std::min(realticketleft[i],ticketleft[i*(tr.locnum-1)+t]);
        }
    }
    queryresult answer;
    answer.trainid=trainid;
    answer.loc1=loc1;
    answer.loc2=loc2;
    answer.time1=timestart[k];
    answer.time2=timearrive[j];
    answer.date1=date1;
    answer.date2=date2;
    answer.ticketkindnum=tr.ticketkindnum;
    for (int i = 0; i < tr.ticketkindnum; i++) {
        answer.ticketKind[i]=tr.ticketKind[i];
        answer.realticketleft[i]=realticketleft[i];
        answer.ticketPrice[i]=ticketPrices[i+tr.ticketkindnum]-ticketPrices[i];
    }
    return answer;

}
int timetonum(const Time& time){
    return ((time[0]-'0')*10+time[1]-'0')*60+(time[3]-'0')*10+time[4]-'0';
}
vector<pair<Trainid , train>> TicketQuery::queryTransfer (const Loc &loc1 , const Loc &loc2 , const int
&date , const
CatalogList &catalogl , Loc &transloc) {
    auto tmp1=loc1;
    auto tmp2=loc2;
    auto catalogList=catalogl.c_str();
    vector<pair<trkey,trval>> trainarray1,trainarray2;
    ticketq.rangeSearch(trkey{loc1 , ""} , trkey{++tmp1 , ""} , trainarray1);
    ticketq.rangeSearch(trkey{loc2 , ""} , trkey{++tmp2 , ""} , trainarray2);
    struct searchKey {
        Loc loc;
        Time timearrive;
        Time timestopover;
        int date;
    };

    struct searchcompare{
        bool operator()(const searchKey& searchKey1,const searchKey& searchKey2)const{
            int cmp;
            cmp=searchKey1.loc.compareTo(searchKey2.loc);
            if (cmp != 0) {
                return cmp<0;
            } else {
                cmp=searchKey1.timearrive.compareTo(searchKey2.timearrive);
                if (cmp != 0) {
                    return cmp<0;
                } else {
                    cmp=searchKey1.timestopover.compareTo(searchKey2.timestopover);
                    if (cmp != 0) {
                        return cmp<0;
                    } else {
                        return searchKey1.date<searchKey2.date;
                    }

                }
            }
        }
    };
    map<searchKey,Trainid,searchcompare> set1;
    map<searchKey,Trainid,searchcompare>set2;
    map<Trainid,int,stringcompare> timetable1;//train1出发时间
    map<Trainid,int,stringcompare> timetable2;//train2到达终点时间
    int currentdate=date;
    bool success;
    for (int i = 0; i < trainarray1.size(); i++) {

        auto tr= trainManager->queryTrain(trainarray1[i].first.trainid , success , nullptr);
        if (tr.ticketleftPos1.GetPageId()==INVALID_PAGE_ID) {
            continue;
        }
        for (char *p = catalogList; *p; p++) {
            if (tr.catalog==*p) {
                goto label1;
            }
        }
        continue;
        label1:




        Tuple ltp_tuple;
        char ltp_buf[3540];
        trainManager->loctimepriceTable->GetTuple(tr.loctimepricePos,ltp_tuple, nullptr);
        ltp_tuple.SerializeTo(ltp_buf);
        auto * loc= reinterpret_cast<Loc*>(ltp_buf);
        auto* timearrive= reinterpret_cast<Time*>(ltp_buf+sizeof(Loc)*tr.locnum);
        auto* timestart= reinterpret_cast<Time*>(ltp_buf+(sizeof(Loc)+sizeof(Time))*tr.locnum);
        auto* timestopover= reinterpret_cast<Time*>(ltp_buf+(sizeof(Loc)+2*sizeof(Time))*tr.locnum);
        bool flag=false;
        for (int j=0;j<tr.locnum;j++) {
            if( j>0 && timetonum(timearrive[j])<timetonum(timestart[j-1]) ) {
                currentdate++;
            }
            if(flag) {
                set1.insert(pair<searchKey , Trainid>(searchKey{loc[j] , timearrive[j] ,
                                                                timestopover[j] , currentdate} ,
                                                      trainarray1[i].first.trainid));
            }
            if (loc[j].compareTo(loc1)==0) {
                timetable1.insert(pair<Trainid,int>(trainarray1[i].first.trainid,currentdate*1440+timetonum
                (timestart[j])));
                flag=true;
            }
        }
    }
    currentdate=date;
    for (int i = 0; i < trainarray2.size(); i++) {
        auto tr= trainManager->queryTrain(trainarray2[i].first.trainid , success , nullptr);
        if (tr.ticketleftPos1.GetPageId()==INVALID_PAGE_ID) {
            continue;
        }
        for (char *p = catalogList; *p; p++) {
            if (tr.catalog==*p) {
                goto label2;
            }
        }
        continue;
        label2:
        Tuple ltp_tuple;
        char ltp_buf[3540];
        trainManager->loctimepriceTable->GetTuple(tr.loctimepricePos,ltp_tuple, nullptr);
        ltp_tuple.SerializeTo(ltp_buf);
        auto * loc= reinterpret_cast<Loc*>(ltp_buf);
        auto* timearrive= reinterpret_cast<Time*>(ltp_buf+sizeof(Loc)*tr.locnum);
        auto* timestart= reinterpret_cast<Time*>(ltp_buf+(sizeof(Loc)+sizeof(Time))*tr.locnum);
        auto* timestopover= reinterpret_cast<Time*>(ltp_buf+(sizeof(Loc)+2*sizeof(Time))*tr.locnum);

        for (int j = 0;j<tr.locnum;j++) {
            if( j>0 && timetonum(timearrive[j])<timetonum(timestart[j-1]) ) {
                currentdate++;
            }
            if (loc[j] == loc2) {
                timetable2.insert(pair<Trainid,int>(trainarray2[i].first.trainid,currentdate*1440+timetonum
                (timearrive[j])));
                break;
            }
            set2.insert(pair<searchKey,Trainid>(searchKey{loc[j],timearrive[j],timestopover[j],currentdate},
                                                trainarray2[i].first.trainid));
        }
    }
    auto it1=set1.begin();
    auto it2=set2.begin();
    pair<Trainid , train> answer1;
    pair<Trainid , train> answer2;

    int dis=100000;
    for (; it1 != set1.end(); it1++) {
        for (it2=set2.begin(); it2 != set2.end(); it2++) {//todo:现在如果我把这个map换成vector是O（n^2),想想看用loc的单调性来变成O（n*logn)
            if(it1->first.loc.compareTo(it2->first.loc)!=0) {
                continue;
            }
            if (it1->first.date*1440+timetonum(it1->first.timearrive) <=
                it2->first.date*1440+timetonum(it2->first.timearrive) + timetonum(it2->first.timestopover)) {
                auto newdis=timetable2[it2->second]-timetable1[it1->second];
                if (newdis < dis &&newdis>=0) {
                    answer1=pair<Trainid,train>(it1->second, trainManager->queryTrain(it1->second , success , nullptr));
                    answer2=pair<Trainid,train>(it2->second, trainManager->queryTrain(it2->second , success , nullptr));
                    dis=newdis;
                    transloc=it1->first.loc;
                }
            }
        }
    }
    vector<pair<Trainid,train>> result;
    if(dis!=100000) {
        result.push_back(answer1);
        result.push_back(answer2);
    }
    return result;
}
