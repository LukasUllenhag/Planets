#pragma once
#include "ConstRandomAccessIterator.h"

template<class T>
class RandomAccessIterator : public ConstRandomAccessIterator<T>
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using reference = T&;
	
	using Base = ConstRandomAccessIterator<T>;
	// Inherit the const iterator's constructors
	using Base::Base;

	// Overload the access operators to return references
	// instead of const references
	[[nodiscard]] T& operator*() const
	{
		return const_cast<T&>(Base::operator*());
	}
	[[nodiscard]] T& operator->() const
	{
		return const_cast<T&>(Base::operator*());
	}
	[[nodiscard]] T& operator[](difference_type index) const
	{
		return const_cast<T&>(Base::operator[](index));
	}

	// Overload all of the operators that would have returned
	// a const iterator and make them return a non-const iterator instead

	RandomAccessIterator& operator++()
	{
		Base::operator++();
		return *this;
	}
	RandomAccessIterator operator++(int)
	{
		RandomAccessIterator temporary = *this;
		Base::operator++();
		return temporary;
	}

	// LegacyBidirectionalIterator operations
	RandomAccessIterator& operator--()
	{
		Base::operator--();
		return *this;
	}
	RandomAccessIterator operator--(int)
	{
		RandomAccessIterator temporary = *this;
		Base::operator--();
		return temporary;
	}

	// LegacyRandomAccessIterator operations
	RandomAccessIterator& operator+=(difference_type value)
	{
		Base::operator+=(value);
		return *this;
	}
	RandomAccessIterator& operator-=(difference_type value)
	{
		Base::operator-=(value);
		return *this;
	}
	[[nodiscard]] RandomAccessIterator operator+(difference_type value) const
	{
		RandomAccessIterator temporary = *this;
		temporary += value;
		return temporary;
	}
	[[nodiscard]] RandomAccessIterator operator-(difference_type value) const
	{
		return (*this) + (-value);
	}
	
	// Since we already overloaded a minus operator, we need to overload all of the minus 
	// operators, even though this one does not return an iterator
	[[nodiscard]] difference_type operator-(const RandomAccessIterator& other) const
	{
		return Base::operator-(other);
	}

};

template<class T>
RandomAccessIterator<T> operator+(ptrdiff_t value,
	RandomAccessIterator<T> iterator)
{
	iterator += value;
	return iterator;
}

template<class T>
RandomAccessIterator<T> operator-(ptrdiff_t value,
	RandomAccessIterator<T> iterator)
{
	iterator -= value;
	return iterator;
}
