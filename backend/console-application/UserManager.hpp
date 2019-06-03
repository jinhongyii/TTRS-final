//
// Created by jinho on 3/10/2019.
//

#ifndef TTRS_BACKEND_USERMANAGER_H
#define TTRS_BACKEND_USERMANAGER_H

#include <fstream>
#include "user.h"

namespace sjtu {

    class UserManager {
        Bptree<int,RID> userindex;
        unsigned int usernum=0;
        TableHeap* userRecord;
        RID usernum_rid;
        Tuple usernum_tuple;
    public:
        friend class Console;
        UserManager():userindex("userindex"){
            auto txn=transactionManager.Begin();
            auto head= reinterpret_cast<HeaderPage*>(recordbpm.FetchPage(HEADER_PAGE_ID));
            page_id_t root;

            if (head->GetRootId("user" , root)) {
                userRecord=new TableHeap(&recordbpm,&logManager,root);
                usernum_rid=RID(userRecord->GetFirstDataPageId(),0);
//                std::cout << usernum_rid.ToString() << std::endl;
                userRecord->GetTuple(usernum_rid,usernum_tuple, txn);
                usernum_tuple.SerializeTo(reinterpret_cast<char*>(&usernum));
                recordbpm.UnpinPage(HEADER_PAGE_ID,false);
            } else {
                userRecord=new TableHeap(&recordbpm,&logManager,txn);
                head->InsertRecord("user", userRecord->GetFirstPageId(),
                                   txn, transactionManager.log_manager_);
                Tuple usernum_tuple;
                usernum_tuple.DeserializeFrom(reinterpret_cast<char*>(&usernum),sizeof(usernum));
                userRecord->InsertTuple(usernum_tuple,usernum_rid,txn);
                recordbpm.UnpinPage(HEADER_PAGE_ID,true);
            }

            transactionManager.Commit(txn);
            delete txn;
        }
        ~UserManager (){
            delete userRecord;
        }

        unsigned long long registerUser(const string<40>& username,const string<20>& password,const string<20> &email,
                                        const string<20>&
                                        phone){
            auto txn=transactionManager.Begin();
            auto newuser= user(username , password , email , phone , usernum+startno);
            Tuple user_tuple;
            user_tuple.DeserializeFrom(reinterpret_cast<char*>(&newuser),sizeof(newuser));
            RID user_rid;
            userRecord->InsertTuple(user_tuple,user_rid,txn);
            userindex.put(newuser.userid , user_rid , txn);
            usernum++;
            usernum_tuple.DeserializeFrom(reinterpret_cast<char*>(&usernum),sizeof(usernum));
            userRecord->UpdateTuple(usernum_tuple,usernum_rid,txn);
            transactionManager.Commit(txn);
            delete txn;
            return newuser.getuserid();
        }
        bool login(unsigned long long userid,const string<20>& password){
            auto userToLogin_tuple= search(userid , nullptr);
            user userToLogin;
            userToLogin_tuple.SerializeTo(reinterpret_cast<char*>(&userToLogin));
            return !(!userToLogin_tuple.IsAllocated() || userToLogin.getpassword() != password);
        }
        //note to delete after use
        Tuple search (unsigned long long userid , Transaction *txn) {
            if (userid >= usernum + startno || userid<startno) {
                return Tuple();
            }
            Tuple user_tuple;
            bool success;
            auto rid=userindex.get(userid,success);
//            std::cout << rid.ToString() << std::endl;
            userRecord->GetTuple(rid,user_tuple, txn);
            return user_tuple;
        }

        bool modify (unsigned long long id , const string<40> &name , const string<20> &password , const string<20>
        &email ,const string<20>& phone){
            auto txn=transactionManager.Begin();
            if ( id>=usernum+startno) {
                transactionManager.Abort(txn);
                delete txn;
                return false;
            }
            auto thisuser_tuple= search(id , txn);
            user thisuser;
            thisuser_tuple.SerializeTo(reinterpret_cast<char*>(&thisuser));
            thisuser.setusername(name);
            thisuser.setemail(email);
            thisuser.setpassword(password);
            thisuser.setphonenum(phone);
            thisuser_tuple.DeserializeFrom(reinterpret_cast<char*>(&thisuser),sizeof(thisuser));
            userRecord->UpdateTuple(thisuser_tuple,thisuser_tuple.GetRid(),txn);
            transactionManager.Commit(txn);
            delete txn;
            return true;
        }
        bool changePriviledge(unsigned long long userid1, unsigned long long userid2,int priviledge){
            auto txn=transactionManager.Begin();
            if ((priviledge!=1 &&priviledge!=2)) {
                transactionManager.Abort(txn);
                delete txn;
                return false;
            }
            auto user1_tuple= search(userid1 , txn);
            user user1,user2;
            user1_tuple.SerializeTo(reinterpret_cast<char*>(&user1));
            if(!user1_tuple.IsAllocated()||(user1.getpriviledge()==1)) {
                transactionManager.Abort(txn);
                delete txn;
                return false;
            }
            auto user2_tuple= search(userid2 , txn);
            user2_tuple.SerializeTo(reinterpret_cast<char*>(&user2));
            if (!user2_tuple.IsAllocated() || (user2.getpriviledge()==2)) {
                transactionManager.Abort(txn);
                delete txn;
                return user2_tuple.IsAllocated()&&priviledge==2;
            }
            user2.setpriviledge(priviledge);
            user2_tuple.DeserializeFrom(reinterpret_cast<char*>(&user2),sizeof(user2));
            userRecord->UpdateTuple(user2_tuple,user2_tuple.GetRid(),
                    txn);
            transactionManager.Commit(txn);
            delete txn;
            return true;
        }


    };

}
#endif //TTRS_BACKEND_USERMANAGER_H
//query_profile 2018
//query_profile 2019
//query_profile 2020
//query_profile 2021
//query_profile 2022
//query_profile 2023
//query_profile 2024
//query_profile 2025
//query_profile 2026
//query_profile 2027
//query_profile 2028