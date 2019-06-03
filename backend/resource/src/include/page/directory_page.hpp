//
// Created by Gabriel on 5/19/2019.
//

/*
 * A directory for table heap, store the status of the pages in the table
 */

//TODO: enable logging

#pragma once

#include "page/page.h"
#include "logging/log_manager.h"
#include "concurrency/transaction.h"

namespace sjtu
{

class Directory
{
public:
	page_id_t pageId;
	int32_t usage;

public:
	Directory(page_id_t _pageId, int32_t _usage) : pageId(_pageId), usage(_usage)
	{}

	static Directory deserializeFrom(char *c)
	{
		page_id_t pageId = *reinterpret_cast<page_id_t *>(c);
		int32_t usage = *reinterpret_cast<int32_t *>(c + 4);
		return Directory(pageId, usage);
	}
};

class DirectoryPage : public Page
{
public:
  void init(page_id_t curPageId, page_id_t nxtPageId, Transaction *txn,
            LogManager *logManager) {
          if (ENABLE_LOGGING) {
              LogRecord logRecord(txn->GetTransactionId(),txn->GetPrevLSN(),
                  LogRecordType::NEWDIRECTORYPAGE,INVALID_LSN);
              lsn_t curlsn=logManager->AppendLogRecord(logRecord);
              txn->SetPrevLSN(curlsn);
              SetLSN(curlsn);

          }
          memcpy(GetData(), &curPageId, 4);
		memcpy(GetData() + 4, &nxtPageId, 4);
	}

private:
	inline int32_t getSize()
	{
		return *reinterpret_cast<int32_t *>(GetData() + 8);
	}

	inline void setSize(int32_t size)
	{
		memcpy(GetData() + 8, &size, 4);
	}

	inline int32_t getSpaceLeft()
	{
		return PAGE_SIZE - 2 * sizeof(page_id_t) - sizeof(int32_t) - sizeof(Directory) * getSize();
	}

public:
	inline page_id_t GetNextPageId()
	{
		return *reinterpret_cast<page_id_t *>(GetData() + 4);
	}

	inline void setNextPageId(page_id_t nxt)
	{
		memcpy(GetData() + 4, &nxt, 4);
	}

public:
	bool insert(const Directory &d)
	{
//		if (getSpaceLeft() < sizeof(d)) return false;
		memcpy(GetData() + PAGE_SIZE - getSpaceLeft(), &d, sizeof(Directory));
		setSize(getSize() + 1);
		return true;
	}

	bool erase(page_id_t pageId) //erase an existing page
	{
		auto p = GetData() + 2 * sizeof(page_id_t) + sizeof(int32_t);
		int32_t size = getSize();
		Directory d(pageId, 0);
		for (auto i = 0; i < size; i++, p += sizeof(Directory))
		{
			auto curPage = *reinterpret_cast<page_id_t *>(p);
			if (curPage == pageId)
			{
				memcpy(p, &d, sizeof(Directory));
				setSize(size - 1);
				return true;
			}
		}
		return false;
	}

	bool update(const Directory &d)
	{
		auto p = GetData() + 2 * sizeof(page_id_t) + sizeof(int32_t);
		int32_t size = getSize();
		for (auto i = 0; i < size; i++, p += sizeof(Directory))
		{
			auto curPage = *reinterpret_cast<page_id_t *>(p);
			if (curPage == d.pageId)
			{
				memcpy(p, &d, sizeof(Directory));
				return true;
			}
		}
		return false;
	}

	page_id_t isEnough(int32_t fileSize)
	{
		auto p = GetData() + 2 * sizeof(page_id_t) + sizeof(int32_t);
		int32_t size = getSize();
		for (auto i = 0; i < size; i++, p += sizeof(Directory))
		{
			Directory cur = Directory::deserializeFrom(p);
			if (PAGE_SIZE - 24 - 8 - cur.usage >= fileSize)
			{
				cur.usage += fileSize + 8;
				memcpy(p, &cur, sizeof(Directory));
				return cur.pageId;
			}
		}
		return INVALID_PAGE_ID;
	}

	Directory getFirst()
	{
		auto offset = 2 * sizeof(page_id_t) + sizeof(int32_t);
		return Directory::deserializeFrom(GetData() + offset);
	}

	bool isFull()
	{
		return getSpaceLeft() < sizeof(Directory);
	}
};

}