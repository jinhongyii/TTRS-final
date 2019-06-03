/**
 * log_manager.h
 * log manager maintain a separate thread that is awaken when the log buffer is
 * full or time out(every X second) to write log buffer's content into disk log
 * file.
 */

#pragma once
#include <algorithm>
#include <condition_variable>
#include <future>
#include <mutex>

#include "disk/disk_manager.h"
#include "logging/log_record.h"

namespace sjtu {

class LogManager {
public:
    friend class LogRecovery;
  LogManager(DiskManager *disk_manager)
      : next_lsn_(0), persistent_lsn_(INVALID_LSN),
        disk_manager_(disk_manager) {
      //todo:在analysis的时候改变next_lsn和next_txn_id
     log_buffer_ = new char[LOG_BUFFER_SIZE];
    flush_buffer_ = new char[LOG_BUFFER_SIZE];
  }

  ~LogManager() {
      flush_all();
    delete[] log_buffer_;
    delete[] flush_buffer_;
    log_buffer_ = nullptr;
    flush_buffer_ = nullptr;
  }
  void flush_all();
  // spawn a separate thread to wake up periodically to flush
  void RunFlushThread();
  void StopFlushThread();

  // append a log record into log buffer
  lsn_t AppendLogRecord(LogRecord &log_record);

  // get/set helper functions
  inline lsn_t GetPersistentLSN() { return persistent_lsn_; }
  inline void SetPersistentLSN(lsn_t lsn) { persistent_lsn_ = lsn; }
  inline char *GetLogBuffer() { return log_buffer_; }
  inline lsn_t get_next_lsn(){ return next_lsn_;}
  inline void set_next_lsn(lsn_t lsn){next_lsn_=lsn;}
private:

  // also remember to change constructor accordingly
  int bytes_written;
  // atomic counter, record the next log sequence number
  lsn_t next_lsn_;
  // log records before & include persistent_lsn_ have been written to disk
  lsn_t persistent_lsn_;
  // log buffer related
  char *log_buffer_;
  char *flush_buffer_;
  bool flush;
  int flush_size;
  // disk manager
  DiskManager *disk_manager_;
};

} // namespace sjtu
