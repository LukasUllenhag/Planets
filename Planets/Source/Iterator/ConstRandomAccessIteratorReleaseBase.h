#pragma once
#include "Container/ContainerDebugInfo.h"

// Instead of inheriting from "ConstRandomAccessIteratorDebugBase" one could inherit
// from this class. By doing so, one optimizes the code, but removes the
// iterator error-checking.
template<class T>
class ConstRandomAccessIteratorReleaseBase
{
protected:
	void SetContainerDebugInfo(ContainerDebugInfo<T>* vectorDebugInfo) noexcept
	{
	}
	bool HasContainer() const noexcept
	{
		return true;
	}
	const ContainerDebugInfo<T>* const GetContainerDebugInfo() const
	{
		return nullptr;
	}
	void AddToContainer() noexcept
	{
	}
	void RemoveFromContainer() noexcept
	{
	}
};