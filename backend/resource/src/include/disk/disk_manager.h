#pragma once
#include <atomic>
#include <fstream>
#include <future>
#include <string>
#include "list.hpp"
#include "common/config.h"

namespace sjtu {

class DiskManager {
public:
  DiskManager(const std::string &db_file);
  ~DiskManager();
  inline bool IsValidPage(page_id_t page_id){ return page_id<next_page_id_;}
  void WritePage(page_id_t page_id, const char *page_data);
  void ReadPage(page_id_t page_id, char *page_data);

  void WriteLog(char *log_data, int size);
  int ReadLog (char *log_data , int size , int offset);

  page_id_t AllocatePage();
  void DeallocatePage(page_id_t page_id);

  int GetNumFlushes() const;
  bool GetFlushState() const;


  void clear();

  int GetFileSize(const std::string &name);

  //add by abclzr
  page_id_t get_checkpoint_pos();
  void flush_checkpoint_pos();
  int GetLogSize();

private:
    friend class BufferPoolManager;
  // stream to write log file
  std::fstream log_io_;
  std::string log_name_;
  // stream to write db file
  std::fstream db_io_;
  std::fstream metadata_io;
  std::string file_name_;
  std::string metadata_name;
  page_id_t next_page_id_;
  bool flush_log_;


};

} // namespace sjtu