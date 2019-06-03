/**
 * hash_table.h
 *
 * Abstract class for hash table implementation
 */

#pragma once

namespace sjtu {

template <typename K, typename V, typename hash> class HashTable {
public:
  HashTable() {}
  virtual ~HashTable() {}
  // lookup and modifier
  virtual bool Find(const K &key, V &value) = 0;
  virtual bool Remove(const K &key) = 0;
  virtual void Insert(const K &key, const V &value) = 0;
  virtual int size()=0;
  virtual void clear()=0;
};

} // namespace sjtu
