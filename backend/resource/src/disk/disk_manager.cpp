#include <assert.h>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <common/logger.h>
#include "disk/disk_manager.h"

namespace sjtu
{

static char *buffer_used = nullptr;

DiskManager::DiskManager(const std::string &db_file)
		: file_name_(db_file), next_page_id_(0), flush_log_(false)

{
	std::string::size_type n = file_name_.find(".");
	if (n == std::string::npos)
	{
		LOG_DEBUG("wrong file format");
		return;
	}
	log_name_ = file_name_.substr(0, n) + ".log";
	metadata_name = file_name_.substr(0, n) + ".meta";
//        log_io_.open(log_name_,std::ios::binary|std::ios::trunc|std::ios::out);
//        log_io_.close();
	log_io_.open(log_name_,
				 std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
	// directory or file does not exist
	if (!log_io_.is_open())
	{
		log_io_.clear();
		// create a new file
		log_io_.open(log_name_, std::ios::binary | std::ios::trunc | std::ios::app |
								std::ios::out);
		log_io_.close();
		// reopen with original mode
		log_io_.open(log_name_, std::ios::binary | std::ios::in | std::ios::app |
								std::ios::out);
	}
	metadata_io.open(metadata_name, std::ios::binary | std::ios::in
									| std::ios::out);
	if (!metadata_io.is_open())
	{
		metadata_io.clear();
		metadata_io.open(metadata_name, std::ios::binary |
										std::ios::trunc | std::ios::out);
		metadata_io.close();
		metadata_io.open(metadata_name,
						 std::ios::binary | std::ios::in | std::ios::out);
	}
	db_io_.open(db_file,
				std::ios::binary | std::ios::in | std::ios::out);
	// directory or file does not exist
	if (!db_io_.is_open() || GetFileSize(file_name_) == 0)
	{
		db_io_.clear();
		// create a new file
		db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
		db_io_.close();
		// reopen with original mode
		db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
		char tmp[PAGE_SIZE]{0};
		WritePage(0, tmp);
		next_page_id_ = 1;
	}
	else
	{
		get_checkpoint_pos();
	}
	//todo:让redo()调用getcheckpointpos()
}

DiskManager::~DiskManager()
{
	//todo:only for test
	log_io_.close();
	db_io_.close();
	metadata_io.close();

}

void DiskManager::WritePage(page_id_t page_id, const char *page_data)
{
	size_t offset = page_id * PAGE_SIZE;
	// set write cursor to offset
	db_io_.seekp(offset);
	db_io_.write(page_data, PAGE_SIZE);
	// check for I/O error

	if (db_io_.bad())
	{
		std::cerr << ("I/O error while writing");
		return;
	}
	// needs to flush to keep disk file in sync
	db_io_.flush();
}

void DiskManager::ReadPage(page_id_t page_id, char *page_data)
{
	int offset = page_id * PAGE_SIZE;
	// check if read beyond file length
	if (offset > GetFileSize(file_name_))
	{
		std::cerr << ("I/O error while reading");
		// std::cerr << "I/O error while reading" << std::endl;
	}
	else
	{
		// set read cursor to offset
		db_io_.seekg(offset);
		db_io_.read(page_data, PAGE_SIZE);
		// if file ends before reading PAGE_SIZE
		int read_count = db_io_.gcount();
		if (read_count < PAGE_SIZE)
		{
			std::cerr << "eof:" << db_io_.eof() << (" Read less than a page\n");
			// std::cerr << "Read less than a page" << std::endl;
			memset(page_data + read_count, 0, PAGE_SIZE - read_count);
		}
	}
}

void DiskManager::WriteLog(char *log_data, int size)
{

	buffer_used = log_data;

	if (size == 0)
		return;

	flush_log_ = true;


	// sequence write
	log_io_.write(log_data, size);

	// check for I/O error
	if (log_io_.bad())
	{
		std::cerr << ("I/O error while writing log");
		return;
	}
	// needs to flush to keep disk file in sync
	log_io_.flush();
	flush_log_ = false;
//        std::cout<<"file_size:"<<GetFileSize(log_name_)<<std::endl;
}

int DiskManager::ReadLog(char *log_data, int size, int offset)
{
	if (offset >= GetFileSize(log_name_))
	{
		// LOG_DEBUG("end of log file");
		// LOG_DEBUG("file size is %d", GetFileSize(log_name_));
		return false;
	}
	log_io_.seekp(offset);
	log_io_.read(log_data, size);
	// if log file ends before reading "size"
	int read_count = log_io_.gcount();
	if (read_count < size)
	{
		log_io_.clear();
		memset(log_data + read_count, 0, size - read_count);
	}

	return read_count;
}

page_id_t DiskManager::AllocatePage()
{
	return next_page_id_++;
}

void DiskManager::DeallocatePage(__attribute__((unused)) page_id_t page_id)
{
	return;
}


bool DiskManager::GetFlushState() const
{ return flush_log_; }

int DiskManager::GetFileSize(const std::string &file_name)
{
	struct stat stat_buf;
	int rc = stat(file_name.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

void DiskManager::clear()
{
	db_io_.close();
	metadata_io.close();
	log_io_.close();
	remove(file_name_.c_str());
	remove(metadata_name.c_str());
	remove(log_name_.c_str());
	new(this) DiskManager(file_name_);
}

int DiskManager::GetLogSize()
{
	return GetFileSize(log_name_);
}

page_id_t DiskManager::get_checkpoint_pos()
{
	if (GetFileSize(metadata_name) == 0)
	{
		next_page_id_ = 1;
		return 0;
	}
	metadata_io.seekg(0);
	metadata_io.read(reinterpret_cast<char *>(&next_page_id_), sizeof
	(next_page_id_));
	int checkpointoffset;
	metadata_io.read(reinterpret_cast<char *>(&checkpointoffset), sizeof
	(checkpointoffset));
	return checkpointoffset;
}

//use before writing checkpoint log
void DiskManager::flush_checkpoint_pos()
{
	metadata_io.seekg(0);
	metadata_io.write(reinterpret_cast<char *>(&next_page_id_), sizeof(next_page_id_));
	int checkpointoffset = GetLogSize();
	metadata_io.write(reinterpret_cast<char *>(&checkpointoffset), sizeof
	(checkpointoffset));
	metadata_io.flush();
}

} // namespace sjtu
