/*
 * buffer_pool_manager.h
 *
 * Functionality: The simplified Buffer Manager interface allows a client to
 * new/delete pages on disk, to read a disk page into the buffer pool and pin
 * it, also to unpin a page in the buffer pool.
 */

#pragma once
#include <list>
#include <mutex>
#include "concurrency/transaction_manager.h"
#include "buffer/lru_replacer.h"
#include "disk/disk_manager.h"
#include "hash/extendible_hash.h"
#include "logging/log_manager.h"
#include "page/page.h"
#include "hash/hash_method.h"

namespace sjtu {
class BufferPoolManager {
public:
  BufferPoolManager(size_t pool_size, DiskManager *disk_manager,
                          LogManager *log_manager = nullptr,
                          TransactionManager* transactionManager= nullptr);

  ~BufferPoolManager();

  Page *FetchPage(page_id_t page_id);

  bool UnpinPage(page_id_t page_id, bool is_dirty);

  bool FlushPage(page_id_t page_id);

  Page *NewPage(page_id_t &page_id);

  bool DeletePage(page_id_t page_id);

  void FlushAllPages();

  void clear();

  void checkpoint();

private:
  size_t pool_size_; // number of pages in buffer pool
  Page *pages_;      // array of pages
  DiskManager *disk_manager_;
  TransactionManager* transactionManager;
  LogManager *log_manager_;
  HashTable<page_id_t, Page *, hash_for_int> *page_table_; // to keep track of pages
  Replacer<Page *, hash_for_Page_pointer> *replacer_;   // to find an unpinned page for replacement
  sjtu::list<Page *> *free_list_; // to find a free page for replacement
};
} // namespace sjtu
