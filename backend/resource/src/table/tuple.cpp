/**
 * tuple.cpp
 */

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include "common/logger.h"
#include "table/tuple.h"

namespace sjtu {

// Copy constructor
Tuple::Tuple(const Tuple &other)
    : allocated_(other.allocated_), rid_(other.rid_), size_(other.size_) {
  // deep copy
  if (allocated_ == true) {
    // LOG_DEBUG("tuple deep copy");
    data_ = new char[size_];
    memcpy(data_, other.data_, size_);
  } else {
    // LOG_DEBUG("tuple shallow copy");
    data_ = other.data_;
  }
}

Tuple &Tuple::operator=(const Tuple &other) {
  allocated_ = other.allocated_;
  rid_ = other.rid_;
  size_ = other.size_;
  // deep copy
  if (allocated_ == true) {
    // LOG_DEBUG("tuple deep copy");
    data_ = new char[size_];
    memcpy(data_, other.data_, size_);
  } else {
    // LOG_DEBUG("tuple shallow copy");
    data_ = other.data_;
  }

  return *this;
}

void Tuple::SerializeTo(char *storage) const {
//  memcpy(storage, &size_, sizeof(int32_t));
  memcpy(storage , data_, size_);
}

void Tuple::DeserializeFrom (const char *storage , int size) {
  // construct a tuple
  this->size_ = size;
  if (this->allocated_)
    delete[] this->data_;
  this->data_ = new char[this->size_];
  memcpy(this->data_, storage, this->size_);
  this->allocated_ = true;
}

} // namespace sjtu
