#pragma once
#include "../Vector.h"

// Two instances of the normal vector class, when stored 
// next to each other in memory will, thanks to the inheritance, 
// have padding between their data. To avoid the padding, this 
// class can be used instead of the normal vector class.
template<class T>
struct BasicTightlyPackedVector3
{
	BasicTightlyPackedVector3(T x, T y, T z)
		:
		x(x),
		y(y),
		z(z)
	{}
	explicit BasicTightlyPackedVector3(const BasicVector3<T>& other)
		:
		x(other.x),
		y(other.y),
		z(other.z)
	{}
	BasicTightlyPackedVector3() = default;

	explicit operator BasicVector3<T>() const
	{
		return BasicVector3<T>(x, y, z);
	}

	T x = (T)0;
	T y = (T)0;
	T z = (T)0;
};

using TightlyPackedVector3 = BasicTightlyPackedVector3<float>;
using TightlyPackedVector3i = BasicTightlyPackedVector3<int>;