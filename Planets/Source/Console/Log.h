#pragma once
#include "LogMutex.h"

#define ENABLE_LOGGING 1

#define CONCATENATE(a, b) CONCATENATE_I(a, b)
#define CONCATENATE_I(a, b) CONCATENATE_II(~, a ## b)
#define CONCATENATE_II(p, res) res

#if ENABLE_LOGGING
#define LOG(message) \
{ \
	std::lock_guard CONCATENATE(lockGuard, __LINE__)(gLogMutex); \
	std::cout << message; \
}
#else
#define LOG(message)
#endif