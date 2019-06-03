/**
 * log_record.h
 * For every write opeartion on table page, you should write ahead a
 * corresponding log record.
 * For EACH log record, HEADER is like (5 fields in common, 20 bytes in totoal)
 *-------------------------------------------------------------
 * | size | LSN | transID | prevLSN | LogType |
 *-------------------------------------------------------------
 * For insert type log record
 *-------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | tuple_data(char[] array) |
 *-------------------------------------------------------------
 * For delete type(including markdelete, rollbackdelete, applydelete)
 *-------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | tuple_data(char[] array) |
 *-------------------------------------------------------------
 * For update type log record
 *------------------------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | old_tuple_data | tuple_size |
 * | new_tuple_data |
 *------------------------------------------------------------------------------
 * For new page type log record
 *-------------------------------------------------------------
 * | HEADER | Directory_page_id |
 *-------------------------------------------------------------
 * For CLR type log record
 * -------------------------------------------------------------
 * | HEADER | undo_next | just_undone |
 * --------------------------------------------------------------
 * For INSERTKEY or DELETEKEY type log record
 * -------------------------------------------------------------
 * | HEADER | currentPage | tuple_size | key_tuple_data(char[] array) |
 * | pos |
 * -------------------------------------------------------------
 * For INSERTPTR or DELETEPTR type log record
 * -------------------------------------------------------------
 * | HEADER | currentPage | ptr | pos |
 * -------------------------------------------------------------
 * For MOVE type log record
 * --------------------------------------------------------------
 * | HEADER | currentPage | src | dest | size |
 * -------------------------------------------------------------
 * For DELETEPAGE type log record
 * -------------------------------------------------------------
 * | HEADER | currentPage |
 * -------------------------------------------------------------
 * For INSERTROOT type log record
 * -------------------------------------------------------------
 * | HEADER | indexname(32+1 bytes) | rootid |
 * -------------------------------------------------------------
 * For UPDATEROOT  type log record
 * ------------------------------------------------------------
 * | HEADER | indexname(32+1 bytes) | rootid | old_root_id |
 * ------------------------------------------------------------
 * For UPDATEKEY type log record
 * -----------------------------------------------------------------------------------------------
 * | HEADER | currentPage | old_key_size | old_key_tuple_data | new_key_size |
 * | new_key_tuple_data | pos |
 * -----------------------------------------------------------------------------------------------
 * For UPDATEPTR type log record
 * ------------------------------------------------------------
 * | HEADER | currentPage | old_ptr | new_ptr | pos |
 * -------------------------------------------------------------
 * For NEWBPTREEPAGE type log record
 * --------------------------------------------------------------------
 * | HEADER | newPageid | parent_id | right_id | isleaf | k_v_type |
 * --------------------------------------------------------------------
 * For NEWDIRECTORYPAGE type log record
 * ---------------------------------------------------------------------
 * | HEADER | nextPageid |
 */
#pragma once
#include <cassert>
#include <string.hpp>

#include "common/config.h"
#include "table/tuple.h"

namespace sjtu {
// log record type
enum class LogRecordType {
  INVALID = 0,
  INSERT,
  MARKDELETE,
  APPLYDELETE,
  UPDATE,
  BEGIN,
  COMMIT,
  ABORT,
  // when create a new page in heap table
  NEWPAGE,
  TXN_END,
  INSERTKEY,
  INSERTPTR,
  DELETEPTR,
  DELETEKEY,
  MOVE,
  DELETEPAGE,//deprecated
  UPDATEROOT,
  CLR,
  UPDATEPTR,
  UPDATEKEY,
  NEWBPTREEPAGE,
  INSERTROOT,
  CHECKPOINT,
  NEWDIRECTORYPAGE
  //todo:add checkpoin type
};
//for test
const std::string logrecordTypeName[30]{"INVALID","INSERT","MARKDELETE","APPLYDELETE","UPDATE","BEGIN","COMMIT",
                                        "ABORT","NEWPAGE","TXN_END","INSERTKEY","INSERTPTR","DELETEPTR","DELETEKEY",
                                        "MOVE","DELETEPAGE","UPDATEROOT",
                                        "CLR","UPDATEPTR","UPDATEKEY",
                                        "NEWBPTREEPAGE","INSERTROOT",
                                        "CHECKPOINT","NEWDIRECTORYPAGE"};
inline std::string inttotype(LogRecordType logRecordType){
    return logrecordTypeName[(int)logRecordType];
}
class LogRecord {
  friend class LogManager;
  friend class TransactionManager;
  friend class LogRecovery;

public:
  LogRecord()
      : size_(0), lsn_(INVALID_LSN), txn_id_(INVALID_TXN_ID),
        prev_lsn_(INVALID_LSN), log_record_type_(LogRecordType::INVALID) {}

  // constructor for Transaction type(BEGIN/COMMIT/ABORT/TXN_END/CHECKPOINT)
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type)
      : size_(HEADER_SIZE), lsn_(INVALID_LSN), txn_id_(txn_id),
        prev_lsn_(prev_lsn), log_record_type_(log_record_type) {
      size_ = HEADER_SIZE;
  }

  // constructor for INSERT/DELETE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            const RID &rid, const Tuple &tuple)
      : lsn_(INVALID_LSN), txn_id_(txn_id), prev_lsn_(prev_lsn),
        log_record_type_(log_record_type) {
    if (log_record_type == LogRecordType::INSERT) {
      insert_rid_ = rid;
      insert_tuple_ = tuple;
    } else {
      assert(log_record_type == LogRecordType::APPLYDELETE ||
             log_record_type == LogRecordType::MARKDELETE );
      delete_rid_ = rid;
      delete_tuple_ = tuple;
    }
    // calculate log record size
    size_ = HEADER_SIZE + sizeof(RID) + sizeof(int32_t) + tuple.GetLength();
  }
  // constructor for UPDATE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            const RID &update_rid, const Tuple &old_tuple,
            const Tuple &new_tuple)
      : lsn_(INVALID_LSN), txn_id_(txn_id), prev_lsn_(prev_lsn),
        log_record_type_(log_record_type), update_rid_(update_rid),
        old_tuple_(old_tuple), new_tuple_(new_tuple) {
    // calculate log record size
    size_ = HEADER_SIZE + sizeof(RID) + old_tuple.GetLength() +
            new_tuple.GetLength() + 2 * sizeof(int32_t);
  }

  //constructor for CLR type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            lsn_t undo_next,lsn_t just_undone):
            size_(HEADER_SIZE), lsn_(INVALID_LSN), txn_id_(txn_id),
            prev_lsn_(prev_lsn), log_record_type_(log_record_type),
            undo_next(undo_next),just_undone(just_undone){
      size_ = HEADER_SIZE + 2*sizeof(lsn_t);
  }
  // constructor for NEWPAGE or CLR or DELETEPAGE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            page_id_t pageId_or_LSN)
      : size_(HEADER_SIZE), lsn_(INVALID_LSN), txn_id_(txn_id),
        prev_lsn_(prev_lsn), log_record_type_(log_record_type),
        directory_page_id_(pageId_or_LSN),undo_next(pageId_or_LSN),
        currentPage(pageId_or_LSN) ,nextPageid(pageId_or_LSN){
    // calculate log record size
    size_ = HEADER_SIZE + sizeof(page_id_t);
  }
  //constructor for INSERTKEY or DELETEKEY type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
          page_id_t currentPage,const Tuple& key,int pos):
          lsn_(INVALID_LSN),txn_id_(txn_id)
          ,prev_lsn_(prev_lsn),log_record_type_(log_record_type),currentPage(currentPage),
          key(key),pos(pos){
      size_=HEADER_SIZE+sizeof(currentPage)+key.GetLength()+2*sizeof(int32_t);
  }
  //constructor for INSERTPTR or DELETEPTR type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            page_id_t currentPage,page_id_t ptr,int pos):lsn_(INVALID_LSN),
            txn_id_(txn_id),prev_lsn_(prev_lsn),
            log_record_type_(log_record_type),currentPage(currentPage),
            ptr(ptr),pos(pos){
      size_=HEADER_SIZE+sizeof(currentPage)+2*sizeof(ptr);
  }
  //constructor for MOVE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            page_id_t currentPage,int src,const RID& dest,int movNum):lsn_(INVALID_LSN),
            txn_id_(txn_id),prev_lsn_(prev_lsn),log_record_type_(log_record_type),
            currentPage(currentPage),src(src),dest(dest),movnum(movNum){
      size_=HEADER_SIZE+sizeof(currentPage)+sizeof(src)+sizeof(dest)+sizeof(movNum);
  }
  //constructor for INSERTROOT type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
            const string<32>& indexname,page_id_t root_id):
      lsn_(INVALID_LSN),txn_id_(txn_id),prev_lsn_(prev_lsn),
      log_record_type_(log_record_type),indexname(indexname),
      rootid(root_id){
    size_=HEADER_SIZE+sizeof(indexname)+sizeof(root_id);
  }
      //constructor for UPDATEROOT type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
          const string<32>& indexname,page_id_t root_id,page_id_t old_root_id):
          lsn_(INVALID_LSN),txn_id_(txn_id),prev_lsn_(prev_lsn),
          log_record_type_(log_record_type),indexname(indexname),
          rootid(root_id),old_root_id(old_root_id){
      size_=HEADER_SIZE+sizeof(indexname)+2*sizeof(root_id);
  }
  //constructor for UPDATEKEY type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
          page_id_t currentPage,const Tuple& oldKey,const Tuple& newKey,int
          pos):
          txn_id_(txn_id),prev_lsn_(prev_lsn),log_record_type_(log_record_type),
          currentPage(currentPage),old_key(oldKey),new_key(newKey),pos(pos){
      size_=HEADER_SIZE+sizeof(currentPage)+3*sizeof(int32_t)+old_key
          .GetLength()+new_key.GetLength();
  }
  //constructot for UPDATEPTR type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
              page_id_t currentPage,page_id_t oldPtr,page_id_t newPtr,int pos):
          txn_id_(txn_id),prev_lsn_(prev_lsn),log_record_type_(log_record_type),
          currentPage(currentPage),old_ptr(oldPtr),new_ptr(newPtr),pos(pos){
      size_=HEADER_SIZE+4*sizeof(page_id_t);
  }
  //constructor for NEWBPTREEPAGE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type,
          page_id_t newpageid,page_id_t parentId,page_id_t rightId,bool isleaf,short k_v_type):
          txn_id_(txn_id),prev_lsn_(prev_lsn),log_record_type_(log_record_type),
          pageId(newpageid),parent(parentId),right(rightId),isleaf(isleaf),k_v_type(k_v_type){
      size_=HEADER_SIZE+3*sizeof(page_id_t)+2*sizeof(int32_t );
  }
  ~LogRecord() {}
    inline RID& GetUpdateRID(){ return update_rid_;}

    inline Tuple& GetOldTuple(){ return old_tuple_;}

    inline Tuple& GetNewTuple(){ return new_tuple_;}

  inline RID &GetDeleteRID() { return delete_rid_; }

  inline Tuple &GetInserteTuple() { return insert_tuple_; }

  inline RID &GetInsertRID() { return insert_rid_; }

  inline page_id_t GetNewPageRecord() { return directory_page_id_; }

  inline int32_t GetSize() { return size_; }

  inline lsn_t GetLSN() { return lsn_; }

  inline txn_id_t GetTxnId() { return txn_id_; }

  inline lsn_t GetPrevLSN() { return prev_lsn_; }

  inline LogRecordType &GetLogRecordType() { return log_record_type_; }

  inline page_id_t getcurPage(){ return currentPage;}

  inline Tuple& getKey(){ return key;}

  inline page_id_t getptr(){ return ptr;}

  inline int getsrc(){ return src;}

  inline RID& getdest(){ return dest;}

  inline int getmovNum (){ return movnum;}

  inline string<32>& getindexname(){ return indexname;}

  inline page_id_t getrootid(){ return rootid;}
  // For debug purpose
  inline std::string ToString() const {
    std::ostringstream os;
    os << "Log["
       << "size:" << size_ << ", "
       << "LSN:" << lsn_ << ", "
       << "transID:" << txn_id_ << ", "
       << "prevLSN:" << prev_lsn_ << ", "
       << "LogType:" << inttotype(log_record_type_) << "]";

    os << "  ";
    switch (log_record_type_) {
        case LogRecordType::INSERT:
            os << "insert_tuplel_rid(" << insert_rid_.GetPageId() << "," << insert_rid_.GetSlotNum()<<") size:" << insert_tuple_.GetLength();
            break;
        case LogRecordType::UPDATE:
            os << "update_tuplel_rid(" << update_rid_.GetPageId() << "," << update_rid_.GetSlotNum()<<") size:" << old_tuple_.GetLength();
            break;
        case LogRecordType::MARKDELETE:
            os << "markdelete_tuplel_rid(" << delete_rid_.GetPageId() << "," << delete_rid_.GetSlotNum()<<") size:" << delete_tuple_.GetLength();
            break;
        case LogRecordType::APPLYDELETE:
            os << "applydelete_tuplel_rid(" << delete_rid_.GetPageId() << "," << delete_rid_.GetSlotNum()<<") size:" << delete_tuple_.GetLength();
            break;
        case LogRecordType::NEWPAGE:
            os << "Directory_page_id: " << directory_page_id_;
            break;
        case LogRecordType::CLR:
            os << "undo_next: " << undo_next << "  just_undone: " << just_undone;
            break;
        case LogRecordType::INSERTKEY:
            os << "currentPage: " << currentPage << " key_tuple_size: " << key.GetLength()
            << " pos: " << pos;
//            if (key.GetLength() == 8) {
//              os<<"key: ("<< *reinterpret_cast<int*>(key.GetData())
//              <<","
//                                                                          ""<<*reinterpret_cast<int*>(key.GetData()+4)<<")";
//            } else {
//              os<<"key:"<<*reinterpret_cast<int*>(key.GetData());
//            }
            break;
        case LogRecordType::DELETEKEY:
            os << "currentPage: " << currentPage << " key_tuple_size: " << key.GetLength()
               << " pos: " << pos;
            break;
        case LogRecordType::INSERTPTR:
            os << "currentPage: " << currentPage << " ptr: " << ptr
               << " pos: " << pos;
            break;
        case LogRecordType::DELETEPTR:
            os << "currentPage: " << currentPage << " ptr: " << ptr
               << " pos: " << pos;
            break;
        case LogRecordType::MOVE:
            os << "currentPage: " << currentPage << " src: " << src
            << " dest(" << dest.GetPageId() << "," << dest.GetSlotNum() << ")"
            << " size: " << movnum;
            break;
        case LogRecordType::DELETEPAGE:
            os << "currentPage: " << currentPage;
            break;
        case LogRecordType::INSERTROOT:
            os << "indexname:" << string(indexname) << " roodid: "<< rootid;
            break;
        case LogRecordType::UPDATEROOT:
            os << "indexname:" << string(indexname) << " roodid: "<< rootid << " old_root_id: " << old_root_id;
            break;
        case LogRecordType::UPDATEKEY:
            os << "currentPage: "<< currentPage << " key_size: " << old_key.GetLength()
            << " pos: " << pos;
            break;
        case LogRecordType::UPDATEPTR:
            os << "currentPage: " << currentPage << " old_ptr: " << old_ptr
            << " new_ptr: " << new_ptr << " pos: " << pos;
            break;
        case LogRecordType::NEWBPTREEPAGE:
            os << "newPageid: " << pageId << " parent_id: " << parent
            << " right_id: " << right << " is_leaf: " << isleaf<<" "
                                                                 "k_v_type:"<<k_v_type;
            break;
        case LogRecordType::NEWDIRECTORYPAGE:
            os << "nextPageID: " << nextPageid;
    }
    return os.str();
  }

private:
  // the length of log record(for serialization, in bytes)
  int32_t size_ = 0;
  // must have fields
  lsn_t lsn_ = INVALID_LSN;
  txn_id_t txn_id_ = INVALID_TXN_ID;
  lsn_t prev_lsn_ = INVALID_LSN;
  LogRecordType log_record_type_ = LogRecordType::INVALID;

  // case1: for delete opeartion, delete_tuple_ for UNDO opeartion
  RID delete_rid_;
  Tuple delete_tuple_;

  // case2: for insert opeartion
  RID insert_rid_;
  Tuple insert_tuple_;

  // case3: for update opeartion
  RID update_rid_;
  Tuple old_tuple_;
  Tuple new_tuple_;

  // case4: for new page opeartion
  page_id_t directory_page_id_ = INVALID_PAGE_ID;

  //case5: for clr operation
  lsn_t just_undone;
  lsn_t undo_next=INVALID_LSN;

  //below are bptree operations
  page_id_t currentPage=INVALID_PAGE_ID;

  int pos;
  //case6: for deletekey or insertkey op
  Tuple key;

  //case7: for deleteptr or insertptr op
  page_id_t ptr=INVALID_PAGE_ID;

  //case8: for move op
  int src=-1;
  RID dest;//pageid | offset
  int movnum=-1;//move how many son ptr or data to another page

  //case9: for updateroot op
  string<32> indexname;
  page_id_t rootid=INVALID_PAGE_ID;
  page_id_t old_root_id=INVALID_PAGE_ID;
  //case10: for update key op
  Tuple old_key,new_key;
  //case 11 for update ptr op
  page_id_t old_ptr,new_ptr;
  const static int HEADER_SIZE = 20;
  //case 12: for new bptree page op
  page_id_t pageId,parent,right;
  int32_t isleaf,k_v_type;
  //case 13:for new directory page op
  page_id_t nextPageid;
}; // namespace sjtu

} // namespace sjtu
