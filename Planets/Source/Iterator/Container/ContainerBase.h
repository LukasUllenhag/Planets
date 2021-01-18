#pragma once
#include "ContainerDebugBase.h"
#include "ContainerReleaseBase.h"
#include "../IteratorMacro.h"

#if ENABLE_ITERATOR_ERROR_CHECKING
template<class T>
using ContainerBase = ContainerDebugBase<T>;
#else
template<class T>
using ContainerBase = ContainerReleaseBase<T>;
#endif