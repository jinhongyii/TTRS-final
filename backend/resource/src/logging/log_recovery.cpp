/**
 * log_recovey.cpp
 */

#include "logging/log_recovery.h"
#include "../../../console-application/TicketManager.h"
#include "../../../console-application/TicketQuery.h"
#include "../../../console-application/TrainManager.h"
#include "../../../console-application/UserManager.hpp"
#include "bptree.hpp"
#include "page/table_page.h"
#include "page/directory_page.hpp"

namespace sjtu {
/*
 * deserialize a log record from log buffer
 * @return: true means deserialize succeed, otherwise can't deserialize cause
 * incomplete log record
 */
bool LogRecovery::DeserializeLogRecord(const char *data,
                                       LogRecord &log_record) {
  memcpy(&log_record, data, LogRecord::HEADER_SIZE);
  int pos = LogRecord::HEADER_SIZE;
  int32_t size;
  switch (log_record.log_record_type_) {
  case LogRecordType::INSERT:
    memcpy(&log_record.insert_rid_, data + pos, sizeof(RID));
    pos += sizeof(RID);
    memcpy(&size, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.insert_tuple_.DeserializeFrom(data + pos, size);
    pos += size;
    break;
  case LogRecordType::UPDATE:
    memcpy(&log_record.update_rid_, data + pos, sizeof(RID));
    pos += sizeof(RID);
    memcpy(&size, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.old_tuple_.DeserializeFrom(data + pos, size);
    pos += size;
    memcpy(&size,data+pos,sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.new_tuple_.DeserializeFrom(data + pos, size);
    pos += size;
    break;
  case LogRecordType::APPLYDELETE:
  case LogRecordType::MARKDELETE:
    memcpy(&log_record.delete_rid_, data + pos, sizeof(RID));
    pos += sizeof(RID);
    memcpy(&size, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.delete_tuple_.DeserializeFrom(data + pos, size);
    pos += size;
    break;
  case LogRecordType::NEWPAGE:
    memcpy(&log_record.directory_page_id_, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    break;
  case LogRecordType::CLR:
    memcpy(&log_record.undo_next, data + pos, sizeof(lsn_t));
    pos += sizeof(lsn_t);
    memcpy(&log_record.just_undone, data + pos, sizeof(lsn_t));
    pos += sizeof(lsn_t);
    break;
  case LogRecordType::INSERTKEY:
  case LogRecordType::DELETEKEY:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&size, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.key.DeserializeFrom(data + pos, size);
    pos += size;
    memcpy(&log_record.pos, data + pos, sizeof(int));
    pos += sizeof(int);
    break;
  case LogRecordType::INSERTPTR:
  case LogRecordType::DELETEPTR:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.ptr, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.pos, data + pos, sizeof(int));
    pos += sizeof(int);
    break;
  case LogRecordType::MOVE:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.src, data + pos, sizeof(int));
    pos += sizeof(int);
    memcpy(&log_record.dest, data + pos, sizeof(RID));
    pos += sizeof(RID);
    memcpy(&log_record.movnum, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    break;
  case LogRecordType::DELETEPAGE:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    break;
  case LogRecordType::INSERTROOT:
    memcpy(&log_record.indexname, data + pos, 33);
    pos += 33;
    memcpy(&log_record.rootid, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    break;
  case LogRecordType::UPDATEROOT:
    memcpy(&log_record.indexname, data + pos, 33);
    pos += 33;
    memcpy(&log_record.rootid, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.old_root_id, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    break;
  case LogRecordType::UPDATEKEY:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&size, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    log_record.old_key.DeserializeFrom(data + pos, size);
    pos += size;
    pos += sizeof(int32_t);
    log_record.new_key.DeserializeFrom(data + pos, size);
    pos += size;
    memcpy(&log_record.pos, data + pos, sizeof(int));
    pos += sizeof(int);
    break;
  case LogRecordType::UPDATEPTR:
    memcpy(&log_record.currentPage, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.old_ptr, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.new_ptr, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.pos, data + pos, sizeof(int));
    pos += sizeof(int);
    break;
  case LogRecordType::NEWBPTREEPAGE:
    memcpy(&log_record.pageId, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.parent, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.right, data + pos, sizeof(page_id_t));
    pos += sizeof(page_id_t);
    memcpy(&log_record.isleaf, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    memcpy(&log_record.k_v_type, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    break;
  case LogRecordType::BEGIN:
    break;
  case LogRecordType::COMMIT:
    break;
  case LogRecordType::TXN_END:
    break;
  case LogRecordType::ABORT:
    break;
  case LogRecordType::INVALID:
    break;
  case LogRecordType::CHECKPOINT:
    break;
  case LogRecordType::NEWDIRECTORYPAGE:
      memcpy(&log_record.nextPageid, data + pos, sizeof(page_id_t));
      pos += sizeof(page_id_t);
    break;
  }
  log_record.size_ = pos;
  return false;
}

/*
 *redo phase on TABLE PAGE level(table/table_page.h)
 *read log file from the beginning to end (you must prefetch log records into
 *log buffer to reduce unnecessary I/O operations), remember to compare page's
 *LSN with log_record's sequence number, and also build active_txn_ table &
 *lsn_mapping_ table
 */

typedef Bptree<int, RID>::InnerNode InnerNode0;
typedef Bptree<trkey, trval>::InnerNode InnerNode1;
typedef Bptree<Trainid, RID>::InnerNode InnerNode2;
typedef Bptree<tikey, tival>::InnerNode InnerNode3;
typedef Bptree<int, RID>::LeafNode LeafNode0;
typedef Bptree<trkey, trval>::LeafNode LeafNode1;
typedef Bptree<Trainid, RID>::LeafNode LeafNode2;
typedef Bptree<tikey, tival>::LeafNode LeafNode3;

off_t LogRecovery::redo(LogRecord &log, off_t log_offset) {
    nxt_lsn=log.lsn_+1;
    nxt_txnid=log.txn_id_+1;
  Page *page = nullptr;
  Page *page2 = nullptr;
  DirectoryPage *directoryPage;
  TablePage *tablePage;
  off_t size = log.size_;
  off_t tmp;
  page_id_t pageid = INVALID_PAGE_ID;
  char *data;
  short isLeaf;
  short k_v_type;
  InnerNode0 *inner0;
  InnerNode1 *inner1;
  InnerNode2 *inner2;
  InnerNode3 *inner3;
  LeafNode0 *leaf0;
  LeafNode1 *leaf1;
  LeafNode2 *leaf2;
  LeafNode3 *leaf3;

  lsn_mapping_.Insert(log.lsn_, log_offset);
  if (active_txn_.count(log.txn_id_)) {
    //    active_txn_[log.txn_id_] = log.lsn_;
  } else {
    active_txn_.insert(log.txn_id_);
    txn_now = new Transaction(log.txn_id_);
    txn_list.insert(txn_now);
  }

  txn_now->GetRecordSet()->push_back(log);
  switch (log.log_record_type_) {
  case LogRecordType::INSERT:
    page = buffer_pool_manager_->FetchPage(log.insert_rid_.GetPageId());
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.insert_rid_.GetPageId(), false);
      return size;
    }
    page->SetLSN(log.lsn_);
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->InsertTuple(log.insert_tuple_, log.insert_rid_, nullptr,
                           nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    buffer_pool_manager_->UnpinPage(log.insert_rid_.GetPageId(), true);
    buffer_pool_manager_->UnpinPage(directoryPage->GetPageId(), true);
    break;
  case LogRecordType::UPDATE:
    page = buffer_pool_manager_->FetchPage(log.update_rid_.GetPageId());
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.update_rid_.GetPageId(), false);
      return size;
    }
    page->SetLSN(log.lsn_);
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->UpdateTuple(log.new_tuple_, log.old_tuple_, log.update_rid_,
                           nullptr, nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    buffer_pool_manager_->UnpinPage(log.update_rid_.GetPageId(), true);
    buffer_pool_manager_->UnpinPage(directoryPage->GetPageId(), true);
    break;
  case LogRecordType::APPLYDELETE:
    page = buffer_pool_manager_->FetchPage(log.delete_rid_.GetPageId());
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.delete_rid_.GetPageId(), false);
      return size;
    }
          page->SetLSN(log.lsn_);
          tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->ApplyDelete(log.delete_rid_, nullptr, nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    buffer_pool_manager_->UnpinPage(log.delete_rid_.GetPageId(), true);
    buffer_pool_manager_->UnpinPage(directoryPage->GetPageId(), true);
    break;
  case LogRecordType::MARKDELETE:
    page = buffer_pool_manager_->FetchPage(log.delete_rid_.GetPageId());
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.delete_rid_.GetPageId(), false);
      return size;
    }
    page->SetLSN(log.lsn_);
          tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->MarkDelete(log.delete_rid_, nullptr, nullptr);
    buffer_pool_manager_->UnpinPage(log.delete_rid_.GetPageId(), true);
    break;
  case LogRecordType::NEWPAGE:
    page = buffer_pool_manager_->NewPage(pageid);
          page->SetLSN(log.lsn_);

          tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->Init(pageid, PAGE_SIZE, log.directory_page_id_, 0, 0);
    directoryPage=
        reinterpret_cast<DirectoryPage*>(buffer_pool_manager_
        ->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->insert(Directory(pageid,0));
    buffer_pool_manager_->UnpinPage(directoryPage->GetPageId(),false);
    buffer_pool_manager_->UnpinPage(pageid, true);
    break;
  case LogRecordType::CLR: {
    if (log.just_undone == -1)
      return size;
    LogRecord undolog;
    get_LogRecord(lsn_mapping_[log.just_undone], undolog);
    int n1, n2 = log.lsn_;
    undo(undolog, n1, n2, nullptr, buffer_pool_manager_, false);
    break;
  }
  case LogRecordType::INSERTKEY:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.insert(log.pos, log.key.GetData());
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.insert(log.pos, log.key.GetData());
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.insert(log.pos, log.key.GetData());
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.insert(log.pos, log.key.GetData());
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::DELETEKEY:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.erase(log.pos);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.erase(log.pos);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.erase(log.pos);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.erase(log.pos);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.erase(log.pos);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.erase(log.pos);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.erase(log.pos);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.erase(log.pos);
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::INSERTPTR:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.insert(log.pos, log.ptr);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.insert(log.pos, log.ptr);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.insert(log.pos, log.ptr);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.insert(log.pos, log.ptr);
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::DELETEPTR:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.erase(log.pos);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.erase(log.pos);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.erase(log.pos);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.erase(log.pos);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.erase(log.pos);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.erase(log.pos);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.erase(log.pos);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.erase(log.pos);
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::MOVE:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    page2 = buffer_pool_manager_->FetchPage(log.dest.GetPageId());
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      buffer_pool_manager_->UnpinPage(log.dest.GetPageId(), false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.move(
            log.src, log.movnum,
            reinterpret_cast<LeafNode0 *>(page2->GetData())->entry,
            log.dest.GetSlotNum());
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.move(
            log.src, log.movnum-1,
            reinterpret_cast<InnerNode0 *>(page2->GetData())->keywords,
            log.dest.GetSlotNum());
        inner0->sons.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode0 *>(page2->GetData())->sons,
            log.dest.GetSlotNum());
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.move(
            log.src, log.movnum,
            reinterpret_cast<LeafNode1 *>(page2->GetData())->entry,
            log.dest.GetSlotNum());
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode1 *>(page2->GetData())->keywords,
            log.dest.GetSlotNum());
        inner1->sons.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode1 *>(page2->GetData())->sons,
            log.dest.GetSlotNum());
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.move(
            log.src, log.movnum,
            reinterpret_cast<LeafNode2 *>(page2->GetData())->entry,
            log.dest.GetSlotNum());
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode2 *>(page2->GetData())->keywords,
            log.dest.GetSlotNum());
        inner2->sons.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode2 *>(page2->GetData())->sons,
            log.dest.GetSlotNum());
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.move(
            log.src, log.movnum,
            reinterpret_cast<LeafNode3 *>(page2->GetData())->entry,
            log.dest.GetSlotNum());
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode3 *>(page2->GetData())->keywords,
            log.dest.GetSlotNum());
        inner3->sons.move(
            log.src, log.movnum,
            reinterpret_cast<InnerNode3 *>(page2->GetData())->sons,
            log.dest.GetSlotNum());
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    buffer_pool_manager_->UnpinPage(log.dest.GetPageId(), true);
    break;
  case LogRecordType::DELETEPAGE:
    break;
  case LogRecordType::INSERTROOT: {
    auto head = reinterpret_cast<HeaderPage *>(
        buffer_pool_manager_->FetchPage((HEADER_PAGE_ID)));
    head->InsertRecord(log.indexname, log.rootid, nullptr, nullptr);
    buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
    break;
  }
  case LogRecordType::UPDATEROOT: {
    auto head = reinterpret_cast<HeaderPage *>(
        buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
    head->UpdateRecord(log.indexname, log.rootid);
    buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
    break;
  }
  case LogRecordType::UPDATEKEY:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.update(log.pos, log.new_key.GetData());
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.update(log.pos, log.new_key.GetData());
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.update(log.pos, log.new_key.GetData());
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.update(log.pos, log.new_key.GetData());
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.update(log.pos, log.new_key.GetData());
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.update(log.pos, log.new_key.GetData());
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.update(log.pos, log.new_key.GetData());
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.update(log.pos, log.new_key.GetData());
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::UPDATEPTR:
    page = buffer_pool_manager_->FetchPage(log.currentPage);
    if (page->GetLSN() >= log.lsn_) {
      buffer_pool_manager_->UnpinPage(log.currentPage, false);
      return size;
    }
          page->SetLSN(log.lsn_);

          data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->right = log.new_ptr;
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.update(log.pos, log.new_ptr);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->right = log.new_ptr;
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.update(log.pos, log.new_ptr);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->right = log.new_ptr;
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.update(log.pos, log.new_ptr);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->right = log.new_ptr;
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.update(log.pos, log.new_ptr);
      }
      break;
    }
    buffer_pool_manager_->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::NEWBPTREEPAGE:
    page = buffer_pool_manager_->NewPage(pageid);
          page->SetLSN(log.lsn_);

          data = page->GetData();
    inner0 = reinterpret_cast<InnerNode0 *>(data);
    inner0->init(pageid, log.parent, log.right, log.isleaf, nullptr, nullptr, log.k_v_type);
    buffer_pool_manager_->UnpinPage(inner0->page_id, true);
    break;
  case LogRecordType::BEGIN:
    break;
  case LogRecordType::COMMIT:
    txn_now->SetState(TransactionState::COMMITTED);
    break;
  case LogRecordType::TXN_END:
    txn_list.erase(txn_now);
    active_txn_.erase(txn_now->GetTransactionId());
    delete txn_now;
    txn_now=0;
    break;
  case LogRecordType::ABORT:
    break;
  case LogRecordType::INVALID:
    break;
      case LogRecordType::NEWDIRECTORYPAGE:
          page = buffer_pool_manager_->NewPage(pageid);
          directoryPage = reinterpret_cast<DirectoryPage *>(page);
          directoryPage->init(pageid, log.nextPageid, nullptr, nullptr);
          buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
          break;
  }
  return size;
}

void LogRecovery::Redo() {
  bool log_flag_ = ENABLE_LOGGING;
  txn_now=0;
  ENABLE_LOGGING = false;
  off_t ckpt_pos = disk_manager_->get_checkpoint_pos();
  off_t log_file_length = disk_manager_->GetLogSize();
  off_t read_all = log_file_length - ckpt_pos;
  off_t read_now = 0;
  LogRecord log;
  while (read_now < read_all) {
    page_id_t tmp = ckpt_pos + read_now;
    get_LogRecord(tmp, log);
    read_now += redo(log, ckpt_pos + read_now);
  }
  ENABLE_LOGGING = log_flag_;
}

/*
 *undo phase on TABLE PAGE level(table/table_page.h)
 *iterate through active txn map and undo each operation
 */
void LogRecovery::Undo() {
  bool log_flag_ = ENABLE_LOGGING;
  ENABLE_LOGGING = false;
  off_t tmp;

  Page *page = nullptr;
  TablePage *tablePage;
  lsn_t last_lsn=nxt_lsn;
  for (auto i=txn_list.begin();i!=txn_list.end();i++) {
    LogRecord last_log = *--((*i)->GetRecordSet()->end());
    if ((*i)->GetState() != TransactionState::COMMITTED) {
      txn_id_t thisid = last_log.txn_id_;
      lsn_t next_undo_id;
      last_lsn = last_log.lsn_;
      while (last_log.txn_id_ == thisid) {
        undo(last_log, next_undo_id, last_lsn, logManager,
             buffer_pool_manager_);
        lsn_mapping_.Find(last_log.undo_next, tmp);
        get_LogRecord(tmp, last_log);
      }
    }
    delete *i;
  }
  nxt_lsn=std::max(last_lsn,nxt_lsn);
  logManager->next_lsn_ = nxt_lsn;
  txn_m->set_next_txn_id(nxt_txnid);
  ENABLE_LOGGING = log_flag_;
  active_txn_.clear();
  txn_list.clear();
  nxt_lsn=0;
  nxt_txnid=0;
  lsn_mapping_.clear();
}

void LogRecovery::get_LogRecord(off_t ofst, LogRecord &log) {
    off_t logsize = disk_manager_->GetLogSize();
  disk_manager_->ReadLog(log_buffer_, PAGE_SIZE, ofst);
  DeserializeLogRecord(log_buffer_, log);
  if (log.log_record_type_ == LogRecordType::CHECKPOINT)
    nxt_lsn = log.lsn_, nxt_txnid = log.txn_id_;
}

void LogRecovery::undo(LogRecord &log, lsn_t &next_undo_id, lsn_t &last_lsn,
                       LogManager *LogM, BufferPoolManager *bpm, bool flag) {
  Page *page = nullptr;
  Page *page2 = nullptr;
  DirectoryPage *directoryPage;
  TablePage *tablePage;
  off_t size = log.size_;
  off_t tmp;
  page_id_t pageid = INVALID_PAGE_ID;
  char *data;
  short isLeaf;
  short k_v_type;
  InnerNode0 *inner0;
  InnerNode1 *inner1;
  InnerNode2 *inner2;
  InnerNode3 *inner3;
  LeafNode0 *leaf0;
  LeafNode1 *leaf1;
  LeafNode2 *leaf2;
  LeafNode3 *leaf3;

  if (log.log_record_type_ == LogRecordType::CLR) {
    next_undo_id = log.undo_next;
    return;
  } else {
    next_undo_id = log.prev_lsn_;
  }

  LogRecord CLRLOG(log.txn_id_, last_lsn, LogRecordType::CLR, next_undo_id,
                   log.lsn_);
  switch (log.log_record_type_) {
  case LogRecordType::INSERT:
    page = bpm->FetchPage(log.insert_rid_.GetPageId());
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->ApplyDelete(log.insert_rid_, nullptr, nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(bpm->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    bpm->UnpinPage(directoryPage->GetPageId(), true);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    tablePage->SetLSN(last_lsn);
    bpm->UnpinPage(log.insert_rid_.GetPageId(), true);
    break;
  case LogRecordType::UPDATE:
    page = bpm->FetchPage(log.update_rid_.GetPageId());
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->UpdateTuple(log.old_tuple_, log.new_tuple_, log.update_rid_,
                           nullptr, nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(bpm->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    bpm->UnpinPage(directoryPage->GetPageId(), true);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    tablePage->SetLSN(last_lsn);
    bpm->UnpinPage(log.update_rid_.GetPageId(), true);
    break;
  case LogRecordType::APPLYDELETE:
    page = bpm->FetchPage(log.delete_rid_.GetPageId());
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->InsertTuple(log.delete_tuple_, log.delete_rid_, nullptr,
                           nullptr);
    directoryPage = reinterpret_cast<DirectoryPage *>(bpm->FetchPage(tablePage->GetDirectoryPageId()));
    directoryPage->update(Directory(tablePage->GetPageId(), PAGE_SIZE-tablePage->GetFreeSpaceSize()));
    bpm->UnpinPage(directoryPage->GetPageId(), true);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    tablePage->SetLSN(last_lsn);
    bpm->UnpinPage(log.delete_rid_.GetPageId(), true);
    break;
  case LogRecordType::MARKDELETE:
    page = bpm->FetchPage(log.delete_rid_.GetPageId());
    tablePage = reinterpret_cast<TablePage *>(page);
    tablePage->CLRinsert(log.delete_rid_, nullptr, LogM, CLRLOG.undo_next,
                         CLRLOG.just_undone);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    tablePage->SetLSN(last_lsn);
    bpm->UnpinPage(log.delete_rid_.GetPageId(), true);
    break;
  case LogRecordType::NEWPAGE:
    break;
  case LogRecordType::CLR:
    break;
  case LogRecordType::INSERTKEY:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.erase(log.pos);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.erase(log.pos);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.erase(log.pos);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.erase(log.pos);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.erase(log.pos);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.erase(log.pos);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.erase(log.pos);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.erase(log.pos);
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::DELETEKEY:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.insert(log.pos, log.key.GetData());
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.insert(log.pos, log.key.GetData());
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.insert(log.pos, log.key.GetData());
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.insert(log.pos, log.key.GetData());
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.insert(log.pos, log.key.GetData());
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::INSERTPTR:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.erase(log.pos);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.erase(log.pos);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.erase(log.pos);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.erase(log.pos);
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::DELETEPTR:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.insert(log.pos, log.ptr);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.insert(log.pos, log.ptr);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.insert(log.pos, log.ptr);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.insert(log.pos, log.ptr);
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::MOVE:
    page = bpm->FetchPage(log.currentPage);
    page2 = bpm->FetchPage(log.dest.GetPageId());
    data = page2->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<LeafNode0 *>(page->GetData())->entry,
                          log.src);
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.move(
            log.dest.GetSlotNum(), log.movnum,
            reinterpret_cast<InnerNode0 *>(page->GetData())->keywords, log.src);
        inner0->sons.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<InnerNode0 *>(page->GetData())->sons,
                          log.src);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<LeafNode1 *>(page->GetData())->entry,
                          log.src);
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.move(
            log.dest.GetSlotNum(), log.movnum,
            reinterpret_cast<InnerNode1 *>(page->GetData())->keywords, log.src);
        inner1->sons.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<InnerNode1 *>(page->GetData())->sons,
                          log.src);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<LeafNode2 *>(page->GetData())->entry,
                          log.src);
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.move(
            log.dest.GetSlotNum(), log.movnum,
            reinterpret_cast<InnerNode2 *>(page->GetData())->keywords, log.src);
        inner2->sons.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<InnerNode2 *>(page->GetData())->sons,
                          log.src);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<LeafNode3 *>(page->GetData())->entry,
                          log.src);
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.move(
            log.dest.GetSlotNum(), log.movnum,
            reinterpret_cast<InnerNode3 *>(page->GetData())->keywords, log.src);
        inner3->sons.move(log.dest.GetSlotNum(), log.movnum,
                          reinterpret_cast<InnerNode3 *>(page->GetData())->sons,
                          log.src);
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    page2->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    bpm->UnpinPage(log.dest.GetPageId(), true);
    break;
  case LogRecordType::DELETEPAGE:
    break;
  case LogRecordType ::INSERTROOT: {
    auto head = reinterpret_cast<HeaderPage *>(bpm->FetchPage(HEADER_PAGE_ID));
    head->DeleteRecord(log.indexname);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    head->SetLSN(last_lsn);
    bpm->UnpinPage(HEADER_PAGE_ID, true);
    break;
  }
  case LogRecordType::UPDATEROOT: {
    auto head = reinterpret_cast<HeaderPage *>(bpm->FetchPage(HEADER_PAGE_ID));
    head->UpdateRecord(log.indexname, log.old_root_id);
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    head->SetLSN(last_lsn);
    bpm->UnpinPage(HEADER_PAGE_ID, true);
    break;
  }
  case LogRecordType::UPDATEKEY:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->entry.update(log.pos, log.old_key.GetData());
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->keywords.update(log.pos, log.old_key.GetData());
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->entry.update(log.pos, log.old_key.GetData());
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->keywords.update(log.pos, log.old_key.GetData());
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->entry.update(log.pos, log.old_key.GetData());
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->keywords.update(log.pos, log.old_key.GetData());
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->entry.update(log.pos, log.old_key.GetData());
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->keywords.update(log.pos, log.old_key.GetData());
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::UPDATEPTR:
    page = bpm->FetchPage(log.currentPage);
    data = page->GetData();
    memcpy(&isLeaf, data, sizeof(short));
    memcpy(&k_v_type, data + sizeof(short), sizeof(short));
    switch (k_v_type) {
    case 0:
      if (isLeaf) {
        leaf0 = reinterpret_cast<LeafNode0 *>(data);
        leaf0->right = log.old_ptr;
      } else {
        inner0 = reinterpret_cast<InnerNode0 *>(data);
        inner0->sons.update(log.pos, log.old_ptr);
      }
      break;
    case 2:
      if (isLeaf) {
        leaf1 = reinterpret_cast<LeafNode1 *>(data);
        leaf1->right = log.old_ptr;
      } else {
        inner1 = reinterpret_cast<InnerNode1 *>(data);
        inner1->sons.update(log.pos, log.old_ptr);
      }
      break;
    case 3:
      if (isLeaf) {
        leaf2 = reinterpret_cast<LeafNode2 *>(data);
        leaf2->right = log.old_ptr;
      } else {
        inner2 = reinterpret_cast<InnerNode2 *>(data);
        inner2->sons.update(log.pos, log.old_ptr);
      }
      break;
    case 9:
      if (isLeaf) {
        leaf3 = reinterpret_cast<LeafNode3 *>(data);
        leaf3->right = log.old_ptr;
      } else {
        inner3 = reinterpret_cast<InnerNode3 *>(data);
        inner3->sons.update(log.pos, log.old_ptr);
      }
      break;
    }
    if (flag) {
      LogM->AppendLogRecord(CLRLOG);
      last_lsn = CLRLOG.lsn_;
    }
    page->SetLSN(last_lsn);
    bpm->UnpinPage(log.currentPage, true);
    break;
  case LogRecordType::NEWBPTREEPAGE:
    bpm->DeletePage(log.pageId);
    break;
  case LogRecordType::BEGIN:
    break;
  case LogRecordType::COMMIT:
    break;
  case LogRecordType::TXN_END:
    break;
  case LogRecordType::ABORT:
    break;
  case LogRecordType::INVALID:
    break;
      case LogRecordType::NEWDIRECTORYPAGE:
          break;
  }
}

bool LogRecovery::cmp::operator()(const txnptr a, const txnptr b) {
  return (long long)a < (long long)b;
}

} // namespace sjtu
