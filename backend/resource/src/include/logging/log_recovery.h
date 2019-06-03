/**
 * recovery_manager.h
 * Read log file from disk, redo and undo
 */

#pragma once
#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "buffer/buffer_pool_manager.h"
#include "logging/log_record.h"
#include "page/table_page.h"
#include "utility.hpp"
#include "hash/extendible_hash.h"
#include "hash/hash_method.h"
#include <set>


namespace sjtu {

    typedef Transaction * txnptr;
    class TransactionManager;
class LogRecovery {
public:
  LogRecovery(DiskManager *disk_manager,
                    BufferPoolManager *buffer_pool_manager, LogManager *logManager1, TransactionManager *txn_m1)
      : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager),
         logManager(logManager1), txn_m(txn_m1) {
    // global transaction through recovery phase
    log_buffer_ = new char[LOG_BUFFER_SIZE];
    txn_now = nullptr;
    nxt_txnid = 0;
    nxt_lsn = 0;
  }

  ~LogRecovery() {
    delete[] log_buffer_;
    log_buffer_ = nullptr;
  }

  off_t redo(LogRecord &, off_t);
  static void undo(LogRecord &, lsn_t &, lsn_t &, LogManager *,
      BufferPoolManager *, bool flag = true);
  void get_LogRecord(off_t, LogRecord &);

  void Redo();
  void Undo();
  bool DeserializeLogRecord(const char *data, LogRecord &log_record);

  struct cmp {
      bool operator()(const txnptr, const txnptr);
  };
private:

  // Don't forget to initialize newly added variable in constructor
  DiskManager *disk_manager_;
  BufferPoolManager *buffer_pool_manager_;
  // maintain active transactions and its corresponds latest lsn
  std::set<txn_id_t> active_txn_;
  // mapping log sequence number to log file offset, for undo purpose
  sjtu::ExtendibleHash<lsn_t, off_t, sjtu::hash_for_int> lsn_mapping_;
  // log buffer related

  char *log_buffer_;
  std::set<txnptr, cmp> txn_list;

public:
  Transaction *txn_now;

private:
  LogManager *logManager;
  TransactionManager *txn_m;

  lsn_t nxt_lsn;
  txn_id_t nxt_txnid;
};

} // namespace sjtu
