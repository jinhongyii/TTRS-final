/**
 * header_page.cpp
 */

#include <cassert>

#include "page/table_page.h"

namespace sjtu {
/**
 * Header related
 */
void TablePage::Init(page_id_t page_id, size_t page_size,
                     page_id_t directoryPageid, LogManager *log_manager,
                     Transaction *txn) {
  memcpy(GetData(), &page_id, 4); // set page_id
  if (ENABLE_LOGGING) {
    LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::NEWPAGE,directoryPageid);
      lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
      txn->SetPrevLSN(cur_lsn);
      SetLSN(cur_lsn);
  }
  SetDirectoryPageId(directoryPageid);
  SetNextPageId(INVALID_PAGE_ID);
  SetFreeSpacePointer(page_size);
  SetTupleCount(0);
}

page_id_t TablePage::GetPageId() {
  return *reinterpret_cast<page_id_t *>(GetData());
}

page_id_t TablePage::GetDirectoryPageId() {
  return *reinterpret_cast<page_id_t *>(GetData() + 8);
}

page_id_t TablePage::GetNextPageId() {
  return *reinterpret_cast<page_id_t *>(GetData() + 12);
}

void TablePage::SetDirectoryPageId(page_id_t prev_page_id) {
  memcpy(GetData() + 8, &prev_page_id, 4);
}

void TablePage::SetNextPageId(page_id_t next_page_id) {
  memcpy(GetData() + 12, &next_page_id, 4);
}

/**
 * Tuple related
 */
    bool TablePage::InsertTuple (const Tuple &tuple , RID &rid , Transaction *txn , LogManager *log_manager) {
  assert(tuple.size_ > 0);
  if (GetFreeSpaceSize() < tuple.size_) {
    return false; // not enough space
  }

  // try to reuse a free slot first
  int i;
  for (i = 0; i < GetTupleCount(); ++i) {
    rid.Set(GetPageId(), i);
    if (GetTupleSize(i) == 0) { // empty slot

      break;
    }
  }

  // no free slot left
  if (i == GetTupleCount() && GetFreeSpaceSize() < tuple.size_ + 8) {
    return false; // not enough space
  }

  SetFreeSpacePointer(GetFreeSpacePointer() -
                      tuple.size_); // update free space pointer first
  memcpy(GetData() + GetFreeSpacePointer(), tuple.data_, tuple.size_);
  SetTupleOffset(i, GetFreeSpacePointer());
  SetTupleSize(i, tuple.size_);
  if (i == GetTupleCount()) {
    rid.Set(GetPageId(), i);
    SetTupleCount(GetTupleCount() + 1);
  }
  // write the log after set rid
  if (ENABLE_LOGGING) {//todo:is this wal?? logging should be moved before

    LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::INSERT,rid,tuple);
    lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
    txn->SetPrevLSN(cur_lsn);
    SetLSN(cur_lsn);
      txn->GetRecordSet()->push_back(logRecord);
  }
  // LOG_DEBUG("Tuple inserted");
  return true;
}
    bool TablePage::MarkDelete (const RID &rid , Transaction *txn , LogManager *log_manager) {
  int slot_num = rid.GetSlotNum();
  if (slot_num >= GetTupleCount()) {
    if (ENABLE_LOGGING) {
      txn->SetState(TransactionState::ABORTED);

    }
    return false;
  }

  int32_t tuple_size = GetTupleSize(slot_num);
  if (tuple_size < 0) {
    if (ENABLE_LOGGING) {
      txn->SetState(TransactionState::ABORTED);
    }
    return false;
  }

  if (ENABLE_LOGGING) {

      Tuple  deletedtuple;
      GetTuple(rid,deletedtuple,txn);
      LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::MARKDELETE,rid,deletedtuple);
      lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
      txn->SetPrevLSN(cur_lsn);
      SetLSN(cur_lsn);
      txn->GetRecordSet()->push_back(logRecord);

  }

  // set tuple size to negative value
  if (tuple_size > 0)
    SetTupleSize(slot_num, -tuple_size);
  return true;
}

bool TablePage::UpdateTuple (const Tuple &new_tuple , Tuple &old_tuple , const RID &rid , Transaction *txn ,
                             LogManager *log_manager) {
  int slot_num = rid.GetSlotNum();
  if (slot_num >= GetTupleCount()) {
    if (ENABLE_LOGGING) {
      txn->SetState(TransactionState::ABORTED);
    }
    return false;
  }
  int32_t tuple_size = GetTupleSize(slot_num); // old tuple size
  if (tuple_size <= 0) {
    if (ENABLE_LOGGING) {
      txn->SetState(TransactionState::ABORTED);
    }
    return false;
  }
  if (GetFreeSpaceSize() < new_tuple.size_ - tuple_size) {
    // should delete/insert because not enough space
    return false;
  }

  // copy out old value
  int32_t tuple_offset =
      GetTupleOffset(slot_num); // the tuple offset of the old tuple
  old_tuple.size_ = tuple_size;
  if (old_tuple.allocated_)
    delete[] old_tuple.data_;
  old_tuple.data_ = new char[old_tuple.size_];
  memcpy(old_tuple.data_, GetData() + tuple_offset, old_tuple.size_);
  old_tuple.rid_ = rid;
  old_tuple.allocated_ = true;

  if (ENABLE_LOGGING) {
    // acquire exclusive lock
    // if has shared lock
      LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::UPDATE,rid,old_tuple,new_tuple);
      lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
      txn->SetPrevLSN(cur_lsn);
      SetLSN(cur_lsn);
      txn->GetRecordSet()->push_back(logRecord);
  }

  // update
  int32_t free_space_pointer =
      GetFreeSpacePointer(); // old pointer to the free space
  assert(tuple_offset >= free_space_pointer);
  memmove(GetData() + free_space_pointer + tuple_size - new_tuple.size_,
          GetData() + free_space_pointer, tuple_offset - free_space_pointer);
  SetFreeSpacePointer(free_space_pointer + tuple_size - new_tuple.size_);
  memcpy(GetData() + tuple_offset + tuple_size - new_tuple.size_,
         new_tuple.data_,
         new_tuple.size_);                 // copy new tuple
  SetTupleSize(slot_num, new_tuple.size_); // update tuple size in slot
  for (int i = 0; i < GetTupleCount();
       ++i) { // update tuple offsets (including the updated one)
    int32_t tuple_offset_i = GetTupleOffset(i);
    if (GetTupleSize(i) > 0 && tuple_offset_i < tuple_offset + tuple_size) {
      SetTupleOffset(i, tuple_offset_i + tuple_size - new_tuple.size_);
    }
  }
  return true;
}


void TablePage::ApplyDelete(const RID &rid, Transaction *txn,
                            LogManager *log_manager) {
  int slot_num = rid.GetSlotNum();
  assert(slot_num < GetTupleCount());
  // the tuple offset of the deleted tuple
  int32_t tuple_offset = GetTupleOffset(slot_num);
  int32_t tuple_size = GetTupleSize(slot_num);
  if (tuple_size < 0) { // commit delete
    tuple_size = -tuple_size;
  } // else: rollback insert op

  // copy out delete value, for undo purpose
  Tuple delete_tuple;
  delete_tuple.size_ = tuple_size;
  delete_tuple.data_ = new char[delete_tuple.size_];
  memcpy(delete_tuple.data_, GetData() + tuple_offset, delete_tuple.size_);
  delete_tuple.rid_ = rid;
  delete_tuple.allocated_ = true;

  if (ENABLE_LOGGING) {
      Tuple deleted;
      GetTuple(rid,deleted,txn);

      LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::APPLYDELETE,rid,deleted);
      lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
      txn->SetPrevLSN(cur_lsn);
      SetLSN(cur_lsn);

  }

  int32_t free_space_pointer =
      GetFreeSpacePointer(); // old pointer to the free space
  assert(tuple_offset >= free_space_pointer);
  memmove(GetData() + free_space_pointer + tuple_size,
          GetData() + free_space_pointer, tuple_offset - free_space_pointer);
  SetFreeSpacePointer(free_space_pointer + tuple_size);
  SetTupleSize(slot_num, 0);
  SetTupleOffset(slot_num, 0); // invalid offset
  for (int i = 0; i < GetTupleCount(); ++i) {
    int32_t tuple_offset_i = GetTupleOffset(i);
    if (GetTupleSize(i) != 0 && tuple_offset_i < tuple_offset) {
      SetTupleOffset(i, tuple_offset_i + tuple_size);
    }
  }
}


void TablePage::CLRinsert(const RID &rid, Transaction *txn,
                          LogManager *log_manager, lsn_t undo_next,
                          lsn_t just_undone) {
  if (ENABLE_LOGGING) {
    // must have already grab the exclusive lock
      Tuple deleted;
      GetTuple(rid,deleted,txn);
      LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::CLR,undo_next,just_undone);
      lsn_t cur_lsn=log_manager->AppendLogRecord(logRecord);
      txn->SetPrevLSN(cur_lsn);
      SetLSN(cur_lsn);
  }

  int slot_num = rid.GetSlotNum();
  assert(slot_num < GetTupleCount());
  int32_t tuple_size = GetTupleSize(slot_num);

  // set tuple size to positive value
  if (tuple_size < 0)
    SetTupleSize(slot_num, -tuple_size);
}



bool TablePage::GetTuple (const RID &rid , Tuple &tuple , Transaction *txn) {
  int slot_num = rid.GetSlotNum();
  if (slot_num >= GetTupleCount()) {
    if (ENABLE_LOGGING)
      txn->SetState(TransactionState::ABORTED);
    return false;
  }
  int32_t tuple_size = GetTupleSize(slot_num);
  if (tuple_size <= 0) {
    if (ENABLE_LOGGING)
      txn->SetState(TransactionState::ABORTED);
    return false;
  }

  if (ENABLE_LOGGING) {
    // acquire shared lock

  }

  int32_t tuple_offset = GetTupleOffset(slot_num);
  tuple.size_ = tuple_size;
  if (tuple.allocated_)
    delete[] tuple.data_;
  tuple.data_ = new char[tuple.size_];
  memcpy(tuple.data_, GetData() + tuple_offset, tuple.size_);
  tuple.rid_ = rid;
  tuple.allocated_ = true;
  return true;
}

/**
 * Tuple iterator
 */
bool TablePage::GetFirstTupleRid(RID &first_rid) {
  for (int i = 0; i < GetTupleCount(); ++i) {
    if (GetTupleSize(i) > 0) { // valid tuple
      first_rid.Set(GetPageId(), i);
      return true;
    }
  }
  // there is no tuple within current page
  first_rid.Set(INVALID_PAGE_ID, -1);
  return false;
}

bool TablePage::GetNextTupleRid(const RID &cur_rid, RID &next_rid) {
  assert(cur_rid.GetPageId() == GetPageId());
  for (auto i = cur_rid.GetSlotNum() + 1; i < GetTupleCount(); ++i) {
    if (GetTupleSize(i) > 0) { // valid tuple
      next_rid.Set(GetPageId(), i);
      return true;
    }
  }
  return false; // End of last tuple
}

/**
 * helper functions
 */

// tuple slots
int32_t TablePage::GetTupleOffset(int slot_num) {
  return *reinterpret_cast<int32_t *>(GetData() + 24 + 8 * slot_num);
}

int32_t TablePage::GetTupleSize(int slot_num) {
  return *reinterpret_cast<int32_t *>(GetData() + 28 + 8 * slot_num);
}

void TablePage::SetTupleOffset(int slot_num, int32_t offset) {
  memcpy(GetData() + 24 + 8 * slot_num, &offset, 4);
}

void TablePage::SetTupleSize(int slot_num, int32_t offset) {
  memcpy(GetData() + 28 + 8 * slot_num, &offset, 4);
}

// free space
int32_t TablePage::GetFreeSpacePointer() {
  return *reinterpret_cast<int32_t *>(GetData() + 16);
}

void TablePage::SetFreeSpacePointer(int32_t free_space_pointer) {
  memcpy(GetData() + 16, &free_space_pointer, 4);
}

// tuple count
int32_t TablePage::GetTupleCount() {
  return *reinterpret_cast<int32_t *>(GetData() + 20);
}

void TablePage::SetTupleCount(int32_t tuple_count) {
  memcpy(GetData() + 20, &tuple_count, 4);
}

// for free space calculation
int32_t TablePage::GetFreeSpaceSize() {
  return GetFreeSpacePointer() - 24 - GetTupleCount() * 8;
}

void TablePage::CLRdelete(const RID &rid, Transaction *txn,
                          LogManager *logManager, lsn_t undo_next,
                          lsn_t just_undone) {
        bool tmp=ENABLE_LOGGING;
        if (ENABLE_LOGGING) {

            Tuple inserted;
            GetTuple(rid,inserted,txn);
            LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::CLR,undo_next,just_undone);
            lsn_t cur_lsn=logManager->AppendLogRecord(logRecord);
            txn->SetPrevLSN(cur_lsn);
            SetLSN(cur_lsn);
            ENABLE_LOGGING=false;
        }
        ApplyDelete(rid,txn,logManager);
        ENABLE_LOGGING=tmp;
    }

    void TablePage::CLRupdate(const RID &rid, Transaction *txn,
                              LogManager *logManager, lsn_t undo_next,
                              const Tuple &prev, lsn_t just_undone) {
        bool tmp=ENABLE_LOGGING;
        Tuple now;
        GetTuple(rid,now,txn);
        if (ENABLE_LOGGING) {

            LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),LogRecordType::CLR,undo_next,just_undone);
            lsn_t cur_lsn=logManager->AppendLogRecord(logRecord);
            txn->SetPrevLSN(cur_lsn);
            SetLSN(cur_lsn);
            ENABLE_LOGGING=false;
        }
        UpdateTuple(prev,now,rid,txn,logManager);
        ENABLE_LOGGING=tmp;
    }
} // namespace sjtu
