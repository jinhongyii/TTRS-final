#pragma once

#include "page/page.h"

#include <cstring>
#include <logging/log_manager.h>
#include "concurrency/transaction.h"
#include "logging/log_manager.h"

namespace sjtu {

class HeaderPage : public Page {
public:
  void Init() { SetRecordCount(0); }

  bool InsertRecord(const std::string & name, const page_id_t root_id, Transaction* txn, LogManager* logManager);
  bool DeleteRecord(const std::string &name);
  bool UpdateRecord(const std::string &name, const page_id_t root_id);
  bool UpdateRecord(int RecordId,page_id_t rootid);

  // return root_id if success
  bool GetRootId(const std::string &name, page_id_t &root_id);
  bool GetRootId(int RecordId,page_id_t& root_id);
  int GetRecordCount();
  int FindRecord(const std::string &name);

private:
  void SetRecordCount(int record_count);
};
} // namespace sjtu
