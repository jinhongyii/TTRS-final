#pragma once

#include "common/rid.h"

namespace sjtu {

class Tuple {
  friend class TablePage;

  friend class TableHeap;

  friend class TableIterator;

public:
  // Default constructor (to create a dummy tuple)
  inline Tuple() : allocated_(false), rid_(RID()), size_(0), data_(nullptr) {}

  // constructor for table heap tuple
  Tuple(RID rid) : allocated_(false), rid_(rid) {}

  // copy constructor, deep copy
  Tuple(const Tuple &other);

  // assign operator, deep copy
  Tuple &operator=(const Tuple &other);

  ~Tuple() {
    if (allocated_)
      delete[] data_;
    allocated_ = false;
    data_ = nullptr;
  }
  // serialize tuple data
  void SerializeTo(char *storage) const;

  // deserialize tuple data(deep copy)
  void DeserializeFrom (const char *storage , int size);

  // return RID of current tuple
  inline RID GetRid() const { return rid_; }

  // Get the address of this tuple in the table's backing store
  inline char *GetData() const { return data_; }

  // Get length of the tuple, including varchar legth
  inline int32_t GetLength() const { return size_; }

  inline bool IsAllocated() { return allocated_; }

  friend class LogManager;
private:
  bool allocated_; // is allocated?
  RID rid_;        // if pointing to the table heap, the rid is valid
  int32_t size_;
  char *data_;
};

} // namespace sjtu
