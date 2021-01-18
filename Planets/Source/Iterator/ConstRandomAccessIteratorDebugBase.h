#pragma once
#include "Container/ContainerDebugInfo.h"

// An iterator class that wants to user error-checking should inherit from this class
template<class T>
class ConstRandomAccessIteratorDebugBase
{
public:
	ConstRandomAccessIteratorDebugBase& operator=(const ConstRandomAccessIteratorDebugBase& other)
	{
		if (HasContainer())
		{
			// Remove the old container if we had one
			RemoveFromContainer();
		}
		if (other.HasContainer())
		{
			mContainerDebugInfo = other.mContainerDebugInfo;
			AddToContainer();
		}

		return *this;
	}
	ConstRandomAccessIteratorDebugBase(const ConstRandomAccessIteratorDebugBase& other)
	{
		*this = other;
	}
	ConstRandomAccessIteratorDebugBase() = default;
protected:
	void SetContainerDebugInfo(ContainerDebugInfo<T>* vectorDebugInfo)
	{
		mContainerDebugInfo = vectorDebugInfo;
	}
	const ContainerDebugInfo<T>* const GetContainerDebugInfo() const
	{
		return mContainerDebugInfo ? mContainerDebugInfo : nullptr;
	}
	void RemoveContainer()
	{
		mContainerDebugInfo = nullptr;
	}
	bool HasContainer() const
	{
		return mContainerDebugInfo;
	}
	void AddToContainer()
	{
		assert(HasContainer());
		mContainerDebugInfo->AddIterator(this);
	}
	void RemoveFromContainer()
	{
		assert(HasContainer());
		mContainerDebugInfo->RemoveIterator(this);
	}
private:
	ContainerDebugInfo<T>* mContainerDebugInfo = nullptr;
	// "ContainerDebugInfo" needs to be able to call RemoveContainer()
	friend class ContainerDebugInfo<T>;
};