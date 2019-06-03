//
// Created by jinho on 4/2/2019.
////
#pragma once
//
#include  "bptree.hpp"
#include"util/array.hpp"
#include "util/exceptions.hpp"
#include"util/map.hpp"
#include"util/MemoryController.h"
#include"util/string.hpp"
#include "util/utility.hpp"
#include "util/vector.hpp"
namespace sjtu {
    typedef string<40> Name;
    typedef string<20> Password;
    typedef string<20> Email;
    typedef string<20> Phone;
    typedef int Userid;
    typedef string<20> Loc;
    typedef string<10> Date;
    typedef string<10> CatalogList;
    typedef char Catalog;
    typedef string<20> TicketKind;
    typedef string<20> Trainid;
    typedef string<5> Time;
    typedef float TicketPrice;

    const int startDate = 1;
    const int endDate = 30;
    const int datenum=30;
    const short defaultTicket = 2000;
    const int startno=2019;
}
