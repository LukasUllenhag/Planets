#pragma once
#include "LogMutex.h"

#define ERROR_LOGGING 1

#define CONCATENATE(a, b) CONCATENATE_I(a, b)
#define CONCATENATE_I(a, b) CONCATENATE_II(~, a ## b)
#define CONCATENATE_II(p, res) res

#if ERROR_LOGGING
#define ERROR_LOG(message) \
{ \
	std::lock_guard CONCATENATE(lockGuard, __LINE__)(gLogMutex); \
	std::cerr \
	<< "{" << std::endl << "Error:" << std::endl << message << std::endl \
	<< std::endl << "Caught at: " << std::endl << "Filename: " << __FILE__ << std::endl \
	<< "Line number: " << __LINE__ << std::endl << "}" << std::endl << std::endl; \
}
#else
#define ERROR_LOG(message)
#endif