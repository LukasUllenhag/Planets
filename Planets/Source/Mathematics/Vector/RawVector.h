#pragma once
#include "Source/Iterator/Container/ContainerBase.h"
#include <sstream>

template<class T, int N>
struct RawVector : public ContainerBase<T>
{
	RawVector(const std::initializer_list<T>& values)
	{
		std::copy(values.begin(), values.end(), this->values);
	}
	RawVector()
	{
		for (int i = 0; i < N; ++i)
		{
			values[i] = (T)0;
		}
	}

	T* GetPointerToData()
	{
		return &values[0];
	}
	const T* GetPointerToData() const
	{
		return &values[0];
	}
	std::string GetString() const
	{
		std::ostringstream oStringStream;
		std::transform(std::begin(values), std::end(values), std::ostream_iterator<std::string>(oStringStream),
			[](T value)
			{
				return std::to_string(value) + ", ";
			});
		std::string string = oStringStream.str();
		// Remove the last two characters ", "
		string.erase(string.size() - 2, 2);

		return "(" + string + ")";
	}
	T values[N];
};

template<class T>
struct RawVector<T, 2> : public ContainerBase<T>
{
	RawVector(T x, T y)
		:
		x(x),
		y(y)
	{}
	RawVector() = default;

	T* GetPointerToData()
	{
		return &x;
	}
	const T* GetPointerToData() const
	{
		return &x;
	}
	std::string GetString() const
	{
		return "x: " + std::to_string(x) + " y: " + std::to_string(y);
	}
	T x = (T)0;
	T y = (T)0;
};

template<class T>
struct RawVector<T, 3> : public ContainerBase<T>
{
	RawVector(T x, T y, T z)
		:
		x(x),
		y(y),
		z(z)
	{}
	RawVector() = default;

	T* GetPointerToData()
	{
		return &x;
	}
	const T* GetPointerToData() const
	{
		return &x;
	}
	std::string GetString() const
	{
		return "x: " + std::to_string(x) + " y: " + std::to_string(y) + " z: " + std::to_string(z);
	}
	T x = (T)0;
	T y = (T)0;
	T z = (T)0;
};

template<class T>
struct RawVector<T, 4> : public ContainerBase<T>
{
	RawVector(T x, T y, T z, T w)
		:
		x(x),
		y(y),
		z(z),
		w(w)
	{}
	RawVector() = default;

	T* GetPointerToData()
	{
		return &x;
	}
	const T* GetPointerToData() const
	{
		return &x;
	}
	std::string GetString() const
	{
		return "x: " + std::to_string(x) + " y: " + std::to_string(y) + " z: " + std::to_string(z) + " w: " + std::to_string(w);
	}
	T x = (T)0;
	T y = (T)0;
	T z = (T)0;
	T w = (T)0;
};