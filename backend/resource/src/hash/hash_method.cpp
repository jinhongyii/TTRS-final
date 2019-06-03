//
// Created by abclzr on 19-5-2.
//

#include "hash/hash_method.h"

size_t sjtu::hash_for_int::operator()(int x) {
    return (x % p + p) % p;
}

size_t sjtu::hash_for_Page_pointer::operator()(Page *t) {
    return (size_t) ((long long ) t / sizeof(Page) % p + p) % p;
}