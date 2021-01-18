#pragma once
#include <mutex>

// This mutex should get locked during logging. It is a recursive mutex,
// since it should be able to get locked multiple times by the same thread.
inline std::recursive_mutex gLogMutex;
