#pragma once
#include "ConstRandomAccessIteratorDebugBase.h"
#include "ConstRandomAccessIteratorReleaseBase.h"
#include "IteratorMacro.h"

#if ENABLE_ITERATOR_ERROR_CHECKING
template<class T>
using ConstRandomAccessIteratorBase = ConstRandomAccessIteratorDebugBase<T>;
#else
template<class T>
using ConstRandomAccessIteratorBase = ConstRandomAccessIteratorReleaseBase<T>;
#endif

#if ENABLE_ITERATOR_ERROR_CHECKING
#define ASSERT_POINTER_IS_GREATER_THAN_BEGIN(pointer) \
	assert(pointer > ConstRandomAccessIteratorDebugBase<T>::GetContainerDebugInfo()->begin)
#define ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(pointer) \
	assert(pointer >= ConstRandomAccessIteratorDebugBase<T>::GetContainerDebugInfo()->begin)
#define ASSERT_POINTER_IS_LESS_THAN_END(pointer) \
	assert(pointer < ConstRandomAccessIteratorDebugBase<T>::GetContainerDebugInfo()->end)
#define ASSERT_POINTER_IS_LEQUAL_TO_END(pointer) \
	assert(pointer <= ConstRandomAccessIteratorDebugBase<T>::GetContainerDebugInfo()->end)
#else
#define ASSERT_POINTER_IS_GREATER_THAN_BEGIN(pointer)
#define ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(pointer)
#define ASSERT_POINTER_IS_LESS_THAN_END(pointer)
#define ASSERT_POINTER_IS_LEQUAL_TO_END(pointer)
#endif
template<class T>
class ConstRandomAccessIterator : public ConstRandomAccessIteratorBase<T>
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = const T*;
	using reference = const T&;

	ConstRandomAccessIterator(const T* pointer, ContainerDebugInfo<T>* vectorDebugInfo)
	{
		Base::SetContainerDebugInfo(vectorDebugInfo);
		Base::AddToContainer();
		assert(pointer);
		ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(pointer);
		ASSERT_POINTER_IS_LEQUAL_TO_END(pointer);
		mPointer = pointer;

	}
	ConstRandomAccessIterator() = default;
	~ConstRandomAccessIterator()
	{
		if (Base::HasContainer())
		{
			Base::RemoveFromContainer();
		}
	}

	[[nodiscard]] bool operator==(const ConstRandomAccessIterator& other) const
	{
		assert(IsCompatible(other));
		return mPointer == other.mPointer;
	}
	[[nodiscard]] bool operator!=(const ConstRandomAccessIterator& other) const
	{
		// The assertion occurs inside operator==
		return !(mPointer == other.mPointer);
	}

	[[nodiscard]] const T& operator*() const
	{
		assert(Base::HasContainer());
		ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(mPointer);
		ASSERT_POINTER_IS_LESS_THAN_END(mPointer);
		return *mPointer;
	}
	[[nodiscard]] const T& operator->() const
	{
		// The assertion occurs inside operator*
		return *(*this);
	}

	ConstRandomAccessIterator& operator++()
	{
		assert(Base::HasContainer());
		ASSERT_POINTER_IS_LESS_THAN_END(mPointer);
		mPointer++;
		return *this;
	}
	ConstRandomAccessIterator operator++(int)
	{
		// The assertion occurs inside operator++
		ConstRandomAccessIterator temporary = *this;
		++(*this);
		return temporary;
	}

	// LegacyBidirectionalIterator operations
	ConstRandomAccessIterator& operator--()
	{
		assert(Base::HasContainer());
		ASSERT_POINTER_IS_GREATER_THAN_BEGIN(mPointer);
		mPointer--;
		return *this;
	}
	ConstRandomAccessIterator operator--(int)
	{
		// The assertion occurs inside operator--
		ConstRandomAccessIterator temporary = *this;
		--(*this);
		return temporary;
	}

	// LegacyRandomAccessIterator operations
	ConstRandomAccessIterator& operator+=(difference_type value)
	{
		assert(Base::HasContainer());
		ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(mPointer + value);
		ASSERT_POINTER_IS_LEQUAL_TO_END(mPointer + value);
		mPointer += value;
		return *this;
	}
	ConstRandomAccessIterator& operator-=(difference_type value)
	{
		// The assertion occurs inside operator+=
		return (*this) += -value;
	}
	[[nodiscard]] ConstRandomAccessIterator operator+(difference_type value) const
	{
		// The assertion occurs inside operator+=
		ConstRandomAccessIterator temporary = *this;
		temporary += value;
		return temporary;
	}
	[[nodiscard]] ConstRandomAccessIterator operator-(difference_type value) const
	{
		// The assertion occurs inside operator+=
		return (*this) + (-value);
	}
	[[nodiscard]] difference_type operator-(const ConstRandomAccessIterator& other) const
	{
		assert(IsCompatible(other));
		return difference_type(mPointer - other.mPointer);
	}

	[[nodiscard]] const T& operator[](difference_type index) const
	{
		assert(Base::HasContainer());
		ASSERT_POINTER_IS_GEQUAL_TO_BEGIN(mPointer);
		ASSERT_POINTER_IS_LESS_THAN_END(mPointer);
		return *(mPointer + index);
	}

	[[nodiscard]] bool operator<(const ConstRandomAccessIterator& other) const
	{
		assert(IsCompatible(other));
		return mPointer < other.mPointer;
	}
	[[nodiscard]] bool operator>(const ConstRandomAccessIterator& other) const
	{
		// The assertion occurs inside operator<
		return other < *this;
	}
	[[nodiscard]] bool operator<=(const ConstRandomAccessIterator& other) const
	{
		// The assertion occurs inside operator<
		return !(*this > other);
	}
	[[nodiscard]] bool operator>=(const ConstRandomAccessIterator& other) const
	{
		// The assertion occurs inside operator<
		return !(*this < other);
	}

private:
	bool IsCompatible(const ConstRandomAccessIterator& other) const
	{
		return Base::GetContainerDebugInfo() == other.GetContainerDebugInfo();
	}
private:
	using Base = ConstRandomAccessIteratorBase<T>;
	friend class ContainerDebugInfo<T>;
	const T* mPointer = nullptr;
};

template<class T>
ConstRandomAccessIterator<T> operator+(ptrdiff_t value,
	ConstRandomAccessIterator<T> iterator)
{
	iterator += value;
	return iterator;
}

template<class T>
ConstRandomAccessIterator<T> operator-(ptrdiff_t value,
	ConstRandomAccessIterator<T> iterator)
{
	iterator -= value;
	return iterator;
}
