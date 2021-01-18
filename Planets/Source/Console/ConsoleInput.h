#pragma once
#include "ConsoleInputMutex.h"

#define ENABLE_CONSOLE_INPUTS 1

#define CONCATENATE(a, b) CONCATENATE_I(a, b)
#define CONCATENATE_I(a, b) CONCATENATE_II(~, a ## b)
#define CONCATENATE_II(p, res) res

#if ENABLE_CONSOLE_INPUTS
#define CIN(x) \
{ \
	std::lock_guard CONCATENATE(lockGuard, __LINE__)(gConsoleInputMutex); \
	std::cin >> x; \
}
#else
#define CIN(x)
#endif
