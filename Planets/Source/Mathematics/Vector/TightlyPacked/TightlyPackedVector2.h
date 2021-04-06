#pragma once
#include "../Vector.h"

// Two instances of the normal vector class, when stored 
// next to each other in memory will, thanks to the inheritance, 
// have padding between their data. To avoid the padding, this 
// class can be used instead of the normal vector class.
template<class T>
struct BasicTightlyPackedVector2
{
	BasicTightlyPackedVector2(T x, T y)
		:
		x(x),
		y(y)
	{}
	explicit BasicTightlyPackedVector2(const BasicVector2<T>& other)
		:
		x(other.x),
		y(other.y)
	{}
	BasicTightlyPackedVector2() = default;

	explicit operator BasicVector2<T>() const
	{
		return BasicVector2<T>(x, y);
	}

	T x = (T)0;
	T y = (T)0;
};

using TightlyPackedVector2 = BasicTightlyPackedVector2<float>;
using TightlyPackedVector2i = BasicTightlyPackedVector2<int>;