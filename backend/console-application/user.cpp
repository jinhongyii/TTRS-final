////
//// Created by jinho on 3/10/2019.
////
//
//#include "user.h"
//namespace sjtu {
//
//    inline void user::setpriviledge (int priviledge) {
//        if (priviledge != 1 && priviledge != 2 && priviledge!=0) {
//            throw InvalidArgumentException();
//        }
//        this->priviledge = priviledge;
//    }
//    inline string<40>& user::getusername () {
//        return username;
//    }
//
//    inline unsigned long long user::getuserid () {
//        return userid;
//    }
//
//    inline void user::setusername (const string<40> &name) {
//        username=name;
//    }
//
//    inline void user::setemail (const string<20> &email) {
//        this->email=email;
//    }
//
//    inline void user::setpassword (const string<20> &password) {
//        this->password=password;
//    }
//
//    inline void user::setphonenum (string<20> phone) {
//        phonenum=phone;
//    }
//
//    inline string<20> &user::getpassword () {
//        return password;
//    }
//
//    inline int user::getpriviledge () {
//        return priviledge;
//    }
//
//    inline unsigned long long user::getaddress () {
//        return (userid-startno)*sizeof(user)+sizeof(long long);
//    }
//
//    inline string<20> &user::getemail () {
//        return email;
//    }
//
//    inline string<20> user::getphone () {
//        return phonenum;
//    }
//    inline std::ostream&operator<<(std::ostream& os,const user& user){
//        os<<user.username<<" "<<user.email<<" "<<user.phonenum<<" "<<user.priviledge;
//        return os;
//    }
//}