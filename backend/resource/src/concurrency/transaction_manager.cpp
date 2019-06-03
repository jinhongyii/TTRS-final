/**
 * transaction_manager.cpp
 *
 */
#include "concurrency/transaction_manager.h"
#include "table/table_heap.h"
#include "../../../console-application/UserManager.hpp"
#include "../../../console-application/TicketQuery.h"
#include "../../../console-application/TrainManager.h"
#include "../../../console-application/TicketManager.h"
#include "bptree.hpp"
#include <cassert>

namespace sjtu {

    Transaction *TransactionManager::Begin() {
        Transaction *txn = new Transaction(next_txn_id_++);

        if (ENABLE_LOGGING && log_manager_) {
            // begin has no prev_lsn
            LogRecord lg(txn->GetTransactionId(), INVALID_LSN, LogRecordType::BEGIN);
            const lsn_t cur_lsn = log_manager_->AppendLogRecord(lg);

            txn->SetPrevLSN(cur_lsn);
        }

        return txn;
    }
    /**
     * need to delete the pointer
     * @param txn
     */
    void TransactionManager::Commit(Transaction *txn) {
        txn->SetState(TransactionState::COMMITTED);

        auto record_set = txn->GetRecordSet();
        while (!record_set->empty()) {
            auto &item = record_set->front();
            ;
            if (item.GetLogRecordType() == LogRecordType::MARKDELETE) {

                auto page= reinterpret_cast<TablePage*>(bufferPoolManager->FetchPage(item.GetDeleteRID().GetPageId()));
                page->ApplyDelete(item.GetDeleteRID(),txn,log_manager_);
                bufferPoolManager->UnpinPage(item.GetDeleteRID().GetPageId(),true);
            }
            record_set->pop_front();
        }
        record_set->clear();

        if (ENABLE_LOGGING && log_manager_) {
            const lsn_t prev_lsn = txn->GetPrevLSN();
            assert (prev_lsn != INVALID_LSN);
            LogRecord lg(txn->GetTransactionId(), prev_lsn, LogRecordType::COMMIT);

            log_manager_->AppendLogRecord(lg);
            if(log_manager_->GetPersistentLSN()<txn->GetPrevLSN()) {
                log_manager_->flush_all();
            }
            LogRecord txn_end(txn->GetTransactionId(),lg.GetLSN(),LogRecordType::TXN_END);
            log_manager_->AppendLogRecord(txn_end);
        }

    }


using namespace sjtu;

typedef Bptree<int, RID>::InnerNode InnerNode0;
typedef Bptree<trkey, trval>::InnerNode InnerNode1;
typedef Bptree<Trainid, RID>::InnerNode InnerNode2;
typedef Bptree<tikey, tival>::InnerNode InnerNode3;
typedef Bptree<int, RID>::LeafNode LeafNode0;
typedef Bptree<trkey, trval>::LeafNode LeafNode1;
typedef Bptree<Trainid, RID>::LeafNode LeafNode2;
typedef Bptree<tikey, tival>::LeafNode LeafNode3;

    void TransactionManager::Abort(Transaction *txn) {
        txn->SetState(TransactionState::ABORTED);

        auto record_set = txn->GetRecordSet();
        if (ENABLE_LOGGING && log_manager_) {
            const lsn_t prev_lsn = txn->GetPrevLSN();
            assert (prev_lsn != INVALID_LSN);
            LogRecord lg(txn->GetTransactionId(), prev_lsn, LogRecordType::ABORT);

            log_manager_->AppendLogRecord(lg);
        }
        if (record_set->empty()) {
          if(ENABLE_LOGGING&&log_manager_) {
            LogRecord logRecord(txn->GetTransactionId() , txn->GetPrevLSN() , LogRecordType::TXN_END);
            log_manager_->AppendLogRecord(logRecord);
          }
          return;
        }
        lsn_t last_lsn = record_set->back().lsn_;
        lsn_t next_undo_id;

        while (!record_set->empty()) {
            auto &item = record_set->back();
            LogRecovery::undo(item, next_undo_id, last_lsn, log_manager_, bufferPoolManager);
            while (!record_set->empty() && record_set->back().lsn_ != next_undo_id)
                record_set->pop_back();
        }
        record_set->clear();
        if(ENABLE_LOGGING&&log_manager_) {
            LogRecord logRecord(txn->GetTransactionId() , txn->GetPrevLSN() , LogRecordType::TXN_END);
            log_manager_->AppendLogRecord(logRecord);
        }

    }
} // namespace sjtu
