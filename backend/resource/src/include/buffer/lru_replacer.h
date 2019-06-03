/**
 * lru_replacer.h
 *
 */

#pragma once

#include "buffer/replacer.h"
#include "hash/extendible_hash.h"
#include "page/page.h"
namespace sjtu {

template <typename T, typename hash_for_T> class LRUReplacer : public Replacer<T, hash_for_T> {
public:

  LRUReplacer (int maxsz);

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();

  void clear();

private:
    ExtendibleHash<T,typename sjtu::list<T>::iterator, hash_for_T> hashtable;
    sjtu::list<T> vals;
    int maxsize;

};

} // namespace sjtu
