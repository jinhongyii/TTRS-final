#pragma once

#include <cstring>
#include <iostream>

#include "common/config.h"

namespace sjtu {

class Page {
  friend class BufferPoolManager;

public:
  Page() { ResetMemory(); }
  ~Page(){};

  inline char *GetData() { return data_; }

  inline page_id_t GetPageId() { return page_id_; }

  inline int GetPinCount() { return pin_count_; }

  inline lsn_t GetLSN() { return *reinterpret_cast<lsn_t *>(GetData() + 4); }
  inline void SetLSN(lsn_t lsn) { memcpy(GetData() + 4, &lsn, 4); }

private:
  // method used by buffer pool manager
  inline void ResetMemory() { memset(data_, 0, PAGE_SIZE); }
  // members
  char data_[PAGE_SIZE]; // actual data
  page_id_t page_id_ = INVALID_PAGE_ID;
  int pin_count_ = 0;
  bool is_dirty_ = false;
};

} // namespace sjtu
