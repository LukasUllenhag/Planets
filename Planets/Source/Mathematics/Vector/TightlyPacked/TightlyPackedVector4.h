#pragma once
#include "../Vector.h"

// Two instances of the normal vector class, when stored 
// next to each other in memory will, thanks to the inheritance, 
// have padding between their data. To avoid the padding, this 
// class can be used instead of the normal vector class.
template<class T>
struct BasicTightlyPackedVector4
{
	BasicTightlyPackedVector4(T x, T y, T z, T w)
		:
		x(x),
		y(y),
		z(z),
		w(w)
	{}
	explicit BasicTightlyPackedVector4(const BasicVector4<T>& other)
		:
		x(other.x),
		y(other.y),
		z(other.z),
		w(other.w)
	{}
	BasicTightlyPackedVector4() = default;

	explicit operator BasicVector4<T>() const
	{
		return BasicVector4<T>(x, y, z, w);
	}

	T x = (T)0;
	T y = (T)0;
	T z = (T)0;
	T w = (T)0;
};

using TightlyPackedVector4 =  BasicTightlyPackedVector4<float>;
using TightlyPackedVector4i = BasicTightlyPackedVector4<int>;