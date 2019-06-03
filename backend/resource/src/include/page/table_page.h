#pragma once

#include <cstring>

#include "common/rid.h"
#include "logging/log_manager.h"
#include "page/page.h"
#include "table/tuple.h"
#include "concurrency/transaction.h"

namespace sjtu
{

class TablePage : public Page
{
public:
	void Init(page_id_t page_id, size_t page_size, page_id_t directoryPageid,
			  LogManager *log_manager, Transaction *txn);

	page_id_t GetPageId();

	page_id_t GetDirectoryPageId();

	page_id_t GetNextPageId();

	void SetDirectoryPageId(page_id_t prev_page_id);

	void SetNextPageId(page_id_t next_page_id);

	bool InsertTuple(const Tuple &tuple, RID &rid, Transaction *txn, LogManager *log_manager); // return rid if success
	bool MarkDelete(const RID &rid, Transaction *txn, LogManager *log_manager); // delete
	bool UpdateTuple(const Tuple &new_tuple, Tuple &old_tuple, const RID &rid, Transaction *txn,
					 LogManager *log_manager);

	// commit/abort time
	void ApplyDelete(const RID &rid, Transaction *txn,
					 LogManager *log_manager); // when commit success
	void CLRinsert(const RID& rid, Transaction* txn, LogManager* log_manager, lsn_t undo_next, lsn_t just_undone); // when commit abort
	void CLRdelete(const RID& rid, Transaction* txn, LogManager* logManager, lsn_t undo_next, lsn_t just_undone);

	void CLRupdate(const RID& rid, Transaction* txn, LogManager* logManager, lsn_t undo_next, const Tuple& prev, lsn_t just_undone);

	// return tuple (with data pointing to heap) if success
	bool GetTuple(const RID &rid, Tuple &tuple, Transaction *txn);

	bool GetFirstTupleRid(RID &first_rid);

	bool GetNextTupleRid(const RID &cur_rid, RID &next_rid);

private:
	int32_t GetTupleOffset(int slot_num);

	int32_t GetTupleSize(int slot_num);

	void SetTupleOffset(int slot_num, int32_t offset);

	void SetTupleSize(int slot_num, int32_t offset);

	int32_t GetFreeSpacePointer(); // offset of the beginning of free space
	void SetFreeSpacePointer(int32_t free_space_pointer);

	int32_t GetTupleCount(); // Note that this tuple count may be larger than # of
	// actual tuples because some slots may be empty
	void SetTupleCount(int32_t tuple_count);

public:
	int32_t GetFreeSpaceSize();
};
} // namespace sjtu
