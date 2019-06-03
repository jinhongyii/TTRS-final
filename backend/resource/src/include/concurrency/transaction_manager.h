/**
 * transaction_manager.h
 *
 */

#pragma once
#include <atomic>
#include <unordered_set>

#include "common/config.h"
#include "logging/log_manager.h"
#include "transaction.h"

namespace sjtu {
    class BufferPoolManager;
    class TransactionManager {
    public:
        TransactionManager(LogManager *log_manager = nullptr,BufferPoolManager* bufferPoolManager= nullptr)
                : next_txn_id_(0),
                  log_manager_(log_manager),bufferPoolManager(bufferPoolManager) {}
        Transaction *Begin();
        void Commit(Transaction *txn);
        void Abort(Transaction *txn);
        txn_id_t get_next_txn_id(){ return next_txn_id_;}
        void set_next_txn_id(txn_id_t txnId){next_txn_id_=txnId;}
    private:
        std::atomic<txn_id_t> next_txn_id_;
        BufferPoolManager* bufferPoolManager;
    public:
        LogManager *log_manager_;
    };

} // namespace sjtu
