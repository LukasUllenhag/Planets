#pragma once
#include "ContainerDebugInfo.h"

// Instead of inheriting from "ContainerDebugBase" one could inherit
// from this class. By doing so, one optimizes the code, but removes the
// iterator error-checking.
template<class T>
class ContainerReleaseBase
{
protected:
	void InitializeContainerDebugInfo(T* begin, T* end) noexcept
	{
	}
	ContainerDebugInfo<T>* GetContainerDebugInfo() noexcept
	{
		return nullptr;
	}
};