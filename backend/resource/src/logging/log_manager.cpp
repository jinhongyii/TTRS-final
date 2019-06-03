/**
 * log_manager.cpp
 */

#include "logging/log_manager.h"
#include "common/logger.h"
namespace sjtu {


    void LogManager::RunFlushThread () {
        ENABLE_LOGGING = true;
    }


    void LogManager::StopFlushThread () {}


    lsn_t LogManager::AppendLogRecord (LogRecord &log_record) {
        log_record.lsn_ = next_lsn_++;
//        LOG_DEBUG(log_record.ToString().c_str());

        if (bytes_written + log_record.size_ >= LOG_BUFFER_SIZE) {
            flush_all();
        }
        memcpy(log_buffer_ + bytes_written , &log_record , LogRecord::HEADER_SIZE);
        int pos = bytes_written + LogRecord::HEADER_SIZE;
        switch (log_record.log_record_type_) {
            case LogRecordType::INSERT:
                memcpy(log_buffer_ + pos , &log_record.insert_rid_ , sizeof(RID));
                pos += sizeof(RID);
                memcpy(log_buffer_ + pos, &log_record.insert_tuple_.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.insert_tuple_.SerializeTo(log_buffer_ + pos);
                break;
            case LogRecordType::UPDATE:
                memcpy(log_buffer_ + pos , &log_record.update_rid_ , sizeof(RID));
                pos += sizeof(RID);
                memcpy(log_buffer_ + pos, &log_record.old_tuple_.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.old_tuple_.SerializeTo(log_buffer_ + pos);
                pos += log_record.old_tuple_.size_;
                memcpy(log_buffer_ + pos, &log_record.new_tuple_.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.new_tuple_.SerializeTo(log_buffer_ + pos);
                break;
            case LogRecordType::APPLYDELETE:
            case LogRecordType::MARKDELETE:
                memcpy(log_buffer_ + pos , &log_record.delete_rid_ , sizeof(RID));
                pos += sizeof(RID);
                memcpy(log_buffer_ + pos, &log_record.delete_tuple_.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.delete_tuple_.SerializeTo(log_buffer_ + pos);
                break;
            case LogRecordType::NEWPAGE:
                memcpy(log_buffer_ + pos , &log_record.directory_page_id_ , sizeof(page_id_t));
                break;
            case LogRecordType ::CLR:
                memcpy(log_buffer_+pos,&log_record.undo_next,sizeof(lsn_t));
                pos += sizeof(lsn_t);
                memcpy(log_buffer_ + pos, &log_record.just_undone, sizeof(lsn_t));
                break;
            case LogRecordType::INSERTKEY:
            case LogRecordType::DELETEKEY:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.key.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.key.SerializeTo(log_buffer_ + pos);
                pos += log_record.key.size_;
                memcpy(log_buffer_ + pos, &log_record.pos, sizeof(int));
                break;
            case LogRecordType::INSERTPTR:
            case LogRecordType::DELETEPTR:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.ptr, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.pos, sizeof(int));
                break;
            case LogRecordType::MOVE:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.src, sizeof(int));
                pos += sizeof(int);
                memcpy(log_buffer_ + pos, &log_record.dest, sizeof(RID));
                pos += sizeof(RID);
                memcpy(log_buffer_ + pos, &log_record.movnum, sizeof(int));
                break;
            case LogRecordType::DELETEPAGE:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                break;
            case LogRecordType::INSERTROOT:
                memcpy(log_buffer_ + pos, &log_record.indexname, 33);
                pos += 33;
                memcpy(log_buffer_ + pos, &log_record.rootid, sizeof(page_id_t));
                break;
            case LogRecordType::UPDATEROOT:
                memcpy(log_buffer_ + pos, &log_record.indexname, 33);
                pos += 33;
                memcpy(log_buffer_ + pos, &log_record.rootid, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.old_root_id, sizeof(page_id_t));
                break;
            case LogRecordType::UPDATEKEY:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.old_key.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.old_key.SerializeTo(log_buffer_ + pos);
                pos += log_record.old_key.size_;
                memcpy(log_buffer_ + pos, &log_record.new_key.size_, sizeof(int32_t));
                pos += sizeof(int32_t);
                log_record.new_key.SerializeTo(log_buffer_ + pos);
                pos += log_record.new_key.size_;
                memcpy(log_buffer_ + pos, &log_record.pos, sizeof(int));
                break;
            case LogRecordType::UPDATEPTR:
                memcpy(log_buffer_ + pos, &log_record.currentPage, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.old_ptr, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.new_ptr, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.pos, sizeof(int));
                break;
            case LogRecordType::NEWBPTREEPAGE:
                memcpy(log_buffer_ + pos, &log_record.pageId, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.parent, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.right, sizeof(page_id_t));
                pos += sizeof(page_id_t);
                memcpy(log_buffer_ + pos, &log_record.isleaf, sizeof(int32_t));
                pos += sizeof(int32_t);
                memcpy(log_buffer_ + pos, &log_record.k_v_type, sizeof(int32_t));
                break;
            case LogRecordType::NEWDIRECTORYPAGE:
                memcpy(log_buffer_ + pos, &log_record.nextPageid, sizeof(page_id_t));
        }
        bytes_written+=log_record.size_;
        if (log_record.log_record_type_ == LogRecordType::COMMIT) {
            flush_all();
        }
        return log_record.lsn_;
    }

    void LogManager::flush_all () {
        if (bytes_written == 0) {
            return;
        }
        std::swap(flush_buffer_,log_buffer_);
        flush_size=bytes_written;
        disk_manager_->WriteLog(flush_buffer_,flush_size);
        flush_size=0;
        //memset(flush_buffer_,0,LOG_BUFFER_SIZE);
        persistent_lsn_=next_lsn_-1;
        bytes_written=0;
    }

} // namespace sjtu
