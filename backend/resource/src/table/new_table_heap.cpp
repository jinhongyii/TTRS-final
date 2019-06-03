//
// Created by Gabriel on 5/19/2019.
//

#include <cassert>
#include "common/logger.h"
#include "table/table_heap.h"
#include "page/directory_page.hpp"

namespace sjtu
{

// open table
TableHeap::TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager, page_id_t first_page_id)
		: buffer_pool_manager_(buffer_pool_manager), log_manager_(log_manager), first_page_id_(first_page_id)
{
//	std::cerr << "FirstPage" << " " << first_page_id_ << std::endl;
}

// create table
TableHeap::TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager, Transaction *txn)
		: buffer_pool_manager_(buffer_pool_manager), log_manager_(log_manager)
{
	auto directory_first_page =
			reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->NewPage(first_page_id_));
	assert(directory_first_page != nullptr);
        directory_first_page->init(first_page_id_, INVALID_PAGE_ID, txn,log_manager_);

	page_id_t newPageId;
	auto targetPage = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(newPageId));
	targetPage->Init(newPageId, PAGE_SIZE, directory_first_page->GetPageId(),
	    log_manager_, txn);
	buffer_pool_manager_->UnpinPage(targetPage->GetPageId(), true);
	directory_first_page->insert(Directory(newPageId, 0));
	buffer_pool_manager_->UnpinPage(directory_first_page->GetPageId(), true);
}

bool TableHeap::InsertTuple(const Tuple &tuple, RID &rid, Transaction *txn)
{
	if (tuple.size_ + 32 > PAGE_SIZE)  // larger than one page size
	{
		txn->SetState(TransactionState::ABORTED);
		return false;
	}

	auto directory_cur_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(first_page_id_));
	if (directory_cur_page == nullptr) //first_page not found
	{
		txn->SetState(TransactionState::ABORTED);
		return false;
	}

	page_id_t targetPageId;

	while ((targetPageId = directory_cur_page->isEnough(tuple.size_)) == INVALID_PAGE_ID)
	{
		auto directory_next_page_id = directory_cur_page->GetNextPageId();
		if (directory_next_page_id != INVALID_PAGE_ID)
		{
			buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), false);
			directory_cur_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(
					directory_next_page_id));
		}
		else break;
	}

	if (targetPageId == INVALID_PAGE_ID)
	{
		directory_cur_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(first_page_id_));
		while (true)
		{
			if (!directory_cur_page->isFull())
			{
				page_id_t newPageId;
				auto targetPage = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(newPageId));
				targetPage->Init(newPageId, PAGE_SIZE,
				    directory_cur_page->GetPageId(),
				    log_manager_, txn);
				buffer_pool_manager_->UnpinPage(targetPage->GetPageId(), true);
				directory_cur_page->insert(Directory(newPageId, 0));
//				buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), true);
				targetPageId = newPageId;
				break;
			}
			auto directory_next_page_id = directory_cur_page->GetNextPageId();
			if (directory_next_page_id != INVALID_PAGE_ID)
			{
				buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), false);
				directory_cur_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(
						directory_next_page_id));
			}
			else break;
		}
		if (targetPageId == INVALID_PAGE_ID) //end of linked list
		{
			page_id_t directory_next_page_id;
			auto directory_new_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->NewPage(
					directory_next_page_id));

			if (directory_new_page == nullptr)
			{
				buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), false);
				txn->SetState(TransactionState::ABORTED);
				return false;
			}

			//setup cur_page
//			std::cout << "NEXT" << directory_next_page_id << " "<<directory_cur_page->GetPageId()<<std::endl;
			directory_cur_page->setNextPageId(directory_next_page_id);
			buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), true);

			//now deals with new page
                        directory_new_page->init(directory_next_page_id,
                                                 INVALID_PAGE_ID,txn,log_manager_);

			//add a tuple in the directory page to represent a new page
			page_id_t newPageId;
			auto targetPage = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(newPageId));
			targetPage->Init(newPageId, PAGE_SIZE,
			    directory_new_page->GetPageId(),
			    log_manager_, txn);
			buffer_pool_manager_->UnpinPage(targetPage->GetPageId(), true);

			directory_new_page->insert(Directory(newPageId, 0));
			directory_cur_page = directory_new_page;

			targetPageId = newPageId;
		}
	}

	//Insert the tuple now
	auto targetPage = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(targetPageId));
	auto ret = targetPage->InsertTuple(tuple, rid, txn, log_manager_);

	directory_cur_page->update(Directory(rid.GetPageId(), PAGE_SIZE - targetPage->GetFreeSpaceSize()));
	buffer_pool_manager_->UnpinPage(directory_cur_page->GetPageId(), true);

//	std::cout << rid.ToString() << std::endl;
	buffer_pool_manager_->UnpinPage(targetPageId, true);
	return ret;
}

bool TableHeap::MarkDelete(const RID &rid, Transaction *txn)
{
	auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
	if (page == nullptr)
	{
		txn->SetState(TransactionState::ABORTED);
		return false;
	}
	page->MarkDelete(rid, txn, log_manager_);
	buffer_pool_manager_->UnpinPage(page->GetPageId(), true);

	//notify the directory
	auto cur = first_page_id_;
	DirectoryPage *curPage;
	while (cur != INVALID_PAGE_ID)
	{
		curPage = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(cur));
		if (curPage->erase(rid.GetPageId()))
		{
			buffer_pool_manager_->UnpinPage(cur, true);
			break;
		}
		cur = curPage->GetNextPageId();
		buffer_pool_manager_->UnpinPage(curPage->GetPageId(), false);
	}
	return true;
}

bool TableHeap::UpdateTuple(const Tuple &tuple, const RID &rid,
							Transaction *txn)
{
	auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
	if (page == nullptr)
	{
		txn->SetState(TransactionState::ABORTED);
		return false;
	}
	Tuple old_tuple;
	bool is_updated = page->UpdateTuple(tuple, old_tuple, rid, txn, log_manager_);

	if (is_updated)
	{
		auto cur = first_page_id_;
		DirectoryPage *curPage;
		Directory d(rid.GetPageId(), PAGE_SIZE - page->GetFreeSpaceSize());
		while (cur != INVALID_PAGE_ID)
		{
			curPage = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(cur));
			if (curPage->update(d))
			{
				buffer_pool_manager_->UnpinPage(cur, true);
				break;
			}
			cur = curPage->GetNextPageId();
			buffer_pool_manager_->UnpinPage(curPage->GetPageId(), false);
		}
	}

	buffer_pool_manager_->UnpinPage(page->GetPageId(), is_updated);
	return is_updated;
}

void TableHeap::ApplyDelete(const RID &rid, Transaction *txn) //TODO
{
	auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
	page->ApplyDelete(rid, txn, log_manager_);
	buffer_pool_manager_->UnpinPage(rid.GetPageId(), true);
}


// called by tuple iterator
bool TableHeap::GetTuple(const RID &rid, Tuple &tuple, Transaction *txn)
{
	auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
	if (page == nullptr)
	{
		txn->SetState(TransactionState::ABORTED);
		return false;
	}
	bool res = page->GetTuple(rid, tuple, txn);
	buffer_pool_manager_->UnpinPage(rid.GetPageId(), false);
	return res;
}

page_id_t TableHeap::GetFirstPageId() const
{
	return first_page_id_;
}

page_id_t TableHeap::GetFirstDataPageId()
{
	auto directory_cur_page = reinterpret_cast<DirectoryPage *>(buffer_pool_manager_->FetchPage(first_page_id_));
	auto ret = directory_cur_page->getFirst().pageId;
	buffer_pool_manager_->UnpinPage(first_page_id_, false);
	return ret;
}

};

