#pragma once

#include <cstdlib>

namespace sjtu {

template <typename T, typename hash_for_T> class Replacer {
public:
  Replacer() {}
  virtual ~Replacer() {}
  virtual void Insert(const T &value) = 0;
  virtual bool Victim(T &value) = 0;
  virtual bool Erase(const T &value) = 0;
  virtual size_t Size() = 0;
  virtual void clear()=0;
};

} // namespace sjtu
