#pragma once
#include "ContainerDebugInfo.h"

// A container that wants to user error-checking for its iterators
// should inherit from this class
template<class T>
class ContainerDebugBase
{
public:
	ContainerDebugBase() = default;
protected:
	// "ContainerDebugBase" stores a "ContainerDebugInfo" that can't be moved nor
	// copied. In order for the operators of the child of this class not to be 
	// implicitly deleted, we therefore need to implement empty copy assignment and 
	// constructor operators.
	ContainerDebugBase(const ContainerDebugBase&){}
	void operator=(const ContainerDebugBase& other){}

	void InitializeContainerDebugInfo(T* begin, T* end)
	{
		assert(!mInitialized);
		mInitialized = true;
		mContainerDebugInfo.begin = begin;
		mContainerDebugInfo.end = end;
	}
	ContainerDebugInfo<T>* GetContainerDebugInfo()
	{
		assert(mInitialized);
		return &mContainerDebugInfo;
	}
private:
	bool mInitialized = false;
	ContainerDebugInfo<T> mContainerDebugInfo;
};
