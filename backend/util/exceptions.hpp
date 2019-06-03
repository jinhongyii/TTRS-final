#pragma once
#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP



#include <cstddef>
#include <cstring>
#include <string>
#include "common/exception.h"
namespace sjtu {


	class index_out_of_bound : public Exception {
		/* __________________________ */
	};

	class runtime_error : public Exception {
		/* __________________________ */
	};

	class invalid_iterator : public Exception {
		/* __________________________ */
	};

	class container_is_empty : public Exception {
		/* __________________________ */
	};

	class NoSuchElementException:public Exception {

	};

	class ContainerIsFullException:public  Exception {

	};
	class NotEnoughMemoryException:public Exception{

	};
	class NoSuchFileException:public Exception{

	};

	class InvalidArgumentException:public Exception{

	};

}
#endif