#include "buffer/buffer_pool_manager.h"

namespace sjtu {

    BufferPoolManager::BufferPoolManager (size_t pool_size ,
                                          DiskManager *disk_manager ,
                                          LogManager *log_manager,
                                          TransactionManager* transactionManager)
            : pool_size_(pool_size) , disk_manager_(disk_manager) ,
              log_manager_(log_manager),transactionManager(transactionManager) {
        // a consecutive memory space for buffer pool
        pages_ = new Page[pool_size_];
        page_table_ = new ExtendibleHash<page_id_t , Page * , hash_for_int>();
        replacer_ = new LRUReplacer<Page * , hash_for_Page_pointer>(0);
        free_list_ = new sjtu::list<Page *>;

        // put all the pages into free list
        for (size_t i = 0; i < pool_size_; ++i) {
            free_list_->push_back(&pages_[i]);
        }
    }


    BufferPoolManager::~BufferPoolManager () {
        checkpoint();
        delete[] pages_;
        delete page_table_;
        delete replacer_;
        delete free_list_;

    }


    Page *BufferPoolManager::FetchPage (page_id_t page_id) {
        Page *page;
        if (page_id == INVALID_PAGE_ID) {
            return nullptr;
        }
        bool exist = page_table_->Find(page_id , page);
        if (exist) {
            page->pin_count_++;
            if (page->pin_count_ == 1) {
                replacer_->Erase(page);
            }
            return page;
        } else {
            if (!free_list_->empty()) {
                page = free_list_->front();
                free_list_->pop_front();
            } else {
                if (replacer_->Size() == 0) {
                    return nullptr;
                } else {
                    replacer_->Victim(page);
                    if (page->is_dirty_) {
                        if (log_manager_ && page->GetLSN() > log_manager_->GetPersistentLSN()) {
                            log_manager_->flush_all();
                        }
                        disk_manager_->WritePage(page->page_id_ , page->data_);
                    }
                    page_table_->Remove(page->page_id_);
                }
            }
            page_table_->Insert(page_id , page);
            disk_manager_->ReadPage(page_id , page->data_);
            page->page_id_ = page_id;
            page->is_dirty_ = 0;
            page->pin_count_ = 1;
            return page;
        }
    }


    bool BufferPoolManager::UnpinPage (page_id_t page_id , bool is_dirty) {
        Page *page;
        if (!page_table_->Find(page_id , page)) {
            return false;
        }
        if (page->pin_count_ <= 0) {
            return false;
        } else {
            page->pin_count_--;
            if (page->pin_count_ == 0) {
                replacer_->Insert(page);
            }
            if (is_dirty) {
                page->is_dirty_ = true;
            }
            return true;
        }
    }


    bool BufferPoolManager::FlushPage (page_id_t page_id) {
        Page *page;
        auto exist = page_table_->Find(page_id , page);
        if (!exist) {
            return false;
        }
//        assert(page_id!=INVALID_PAGE_ID);
        if (page->is_dirty_) {
            disk_manager_->WritePage(page_id , page->data_);
            if (log_manager_ && page->GetLSN() > log_manager_->GetPersistentLSN()) {
                log_manager_->flush_all();
            }
        }
        page->is_dirty_ = 0;
        return true;
    }


    bool BufferPoolManager::DeletePage (page_id_t page_id) {
        Page *page;
        if (!page_table_->Find(page_id , page)) {
            disk_manager_->DeallocatePage(page_id);
        } else {
            if (page->pin_count_ != 0) {
                return false;
            } else {
                replacer_->Erase(page);
                page_table_->Remove(page_id);
                free_list_->push_back(page);
                disk_manager_->DeallocatePage(page_id);
            }
        }
        page->page_id_ = INVALID_PAGE_ID;
        return true;

    }


    Page *BufferPoolManager::NewPage (page_id_t &page_id) {
        page_id = disk_manager_->AllocatePage();
        char pagedata[PAGE_SIZE]{0};
        disk_manager_->WritePage(page_id , pagedata);
        Page *page;
        if (!free_list_->empty()) {
            page = free_list_->front();
            free_list_->pop_front();

        } else {
            if (replacer_->Size() == 0) {
                return nullptr;
            } else {
                replacer_->Victim(page);
                if (page->is_dirty_) {
                    if (log_manager_ && page->GetLSN() > log_manager_->GetPersistentLSN()) {
                        log_manager_->flush_all();
                    }
                    disk_manager_->WritePage(page->page_id_ , page->data_);
                }

                page_table_->Remove(page->page_id_);
            }

        }
        page->ResetMemory();
        page->page_id_ = page_id;
        page->is_dirty_ = 0;
        page->pin_count_ = 1;
        page_table_->Insert(page_id , page);
        return page;
    }

    void BufferPoolManager::FlushAllPages () {
        for (size_t i = 0; i < pool_size_; i++) {
            FlushPage(pages_[i].page_id_);
        }
    }

    void BufferPoolManager::clear () {
        page_table_->clear();
        replacer_->clear();
        free_list_->clear();
        for (size_t i = 0; i < pool_size_; i++) {
            free_list_->push_back(&pages_[i]);
        }
    }
    void BufferPoolManager::checkpoint() {
        disk_manager_->flush_checkpoint_pos();
        FlushAllPages();
        if (ENABLE_LOGGING) {
            LogRecord logRecord(transactionManager->get_next_txn_id(),
                log_manager_->get_next_lsn()+1,
                LogRecordType::CHECKPOINT);
            log_manager_->AppendLogRecord(logRecord);
            log_manager_->flush_all();
        }
    }
    } // namespace sjtu
