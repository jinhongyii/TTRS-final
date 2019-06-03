//
// Created by abclzr on 19-5-2.
//

#ifndef TTRS_2_HASH_METHOD_H
#define TTRS_2_HASH_METHOD_H

#include <cstddef>
#include "page/page.h"

const int p = 2003;//todo:change to parameter

namespace sjtu {

    class hash_for_int {
    public:
        size_t operator()(int);
    };

    class hash_for_Page_pointer {
    public:
        size_t operator()(Page *);
    };

}
#endif //TTRS_2_HASH_METHOD_H
