/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"
#include "hash/hash_method.h"

namespace sjtu {

template <typename T, typename hash_for_T>
LRUReplacer<T, hash_for_T>::LRUReplacer (int maxsz):hashtable(),maxsize(maxsz) {
}

template <typename T, typename hash_for_T> LRUReplacer<T, hash_for_T>::~LRUReplacer() = default;

/*
 * Insert value into LRU
 */
template <typename T, typename hash_for_T> void LRUReplacer<T, hash_for_T>::Insert(const T &value) {
    auto tmp=vals.begin();
    auto exists=hashtable.Find(value,tmp);
        if (exists) {
            Erase(value);
        }
    vals.push_front(value);
    hashtable.Insert(value,vals.begin());
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T, typename hash_for_T> bool LRUReplacer<T, hash_for_T>::Victim(T &value) {
        if (Size() == 0) {
            return false;
        }
        value=vals.back();
        vals.pop_back();
        hashtable.Remove(value);
        return true;
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T, typename hash_for_T> bool LRUReplacer<T, hash_for_T>::Erase(const T &value) {
        auto i=vals.begin();
        auto tmp=hashtable.Find(value,i);
        if (!tmp) {
            return false;
        }
        vals.erase(i);
        return hashtable.Remove(value);
}

template <typename T, typename hash_for_T> size_t LRUReplacer<T, hash_for_T>::Size() { return hashtable.size(); }

    template<typename T, typename hash_for_T>
    void LRUReplacer<T, hash_for_T>::clear () {
        hashtable.clear();
        vals.clear();
    }


    template class LRUReplacer<Page *, hash_for_Page_pointer>;
// test only
template class LRUReplacer<int, hash_for_int>;

} // namespace sjtu
