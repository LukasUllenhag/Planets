#pragma once
#include "RawVector.h"
#include "Source/Iterator/RandomAccessIterator.h"

template<class T, int N>
requires (N >= 2 && std::is_arithmetic_v<T>)
class BasicVector : public RawVector<T, N>
{
public:
	using Iterator = RandomAccessIterator<T>;
	using ConstIterator = ConstRandomAccessIterator<T>;
	template<class... ArgTypes>
	requires(sizeof...(ArgTypes) == N)
	BasicVector(ArgTypes... arguments)
		:
		Base{ arguments... }
	{
		Base::InitializeContainerDebugInfo(Base::GetPointerToData(), Base::GetPointerToData() + N);
	}

	// Any time we construct a "BasicVector" we need to initialize the container info, hence
	// we overload the copy and default constructors
	BasicVector()
	{
		Base::InitializeContainerDebugInfo(Base::GetPointerToData(), Base::GetPointerToData() + N);
	}
	BasicVector(const BasicVector& other)
		:
		Base(other)
	{
		Base::InitializeContainerDebugInfo(Base::GetPointerToData(), Base::GetPointerToData() + N);
	}
	template<class TOther>
	explicit BasicVector(const BasicVector<TOther, N>& other)
	{
		Base::InitializeContainerDebugInfo(Base::GetPointerToData(), Base::GetPointerToData() + N);
		std::transform(other.begin(), other.end(), begin(),
			[](const TOther value)
			{
				return T(value);
			});
	}
	
	[[nodiscard]] Iterator begin()
	{
		return Iterator(Base::GetPointerToData(), Base::GetContainerDebugInfo());
	}
	[[nodiscard]] ConstIterator begin() const
	{
		// We remove the const from "*this", so that the const iterator can access and modify the 
		// container debug info. Container debug info is safe to modify for const vectors, since 
		// it can not actually change the data of the vector
		return ConstIterator(Base::GetPointerToData(), const_cast<BasicVector&>(*this).GetContainerDebugInfo());
	}
	[[nodiscard]] Iterator end()
	{
		return Iterator(Base::GetPointerToData()+N, Base::GetContainerDebugInfo());
	}
	[[nodiscard]] ConstIterator end() const
	{
		// We remove the const from "*this", so that the const iterator can access and modify the 
		// container debug info. Container debug info is safe to modify for const vectors, since 
		// it can not actually change the data of the vector
		return ConstIterator(Base::GetPointerToData() + N, const_cast<BasicVector&>(*this).GetContainerDebugInfo());
	}

	[[nodiscard]] T& operator[](const size_t index)
	{
		assert(index >= 0 && index < N);
		return Base::GetPointerToData()[index];
	}
	[[nodiscard]] const T& operator[](const size_t index) const
	{
		return const_cast<BasicVector&>(*this)[index];
	}

	[[nodiscard]] T GetLengthSquared() const
	{
		T lengthSquared = (T)0;
		for (int i = 0; i < N; ++i)
		{
			lengthSquared += (T)pow((double)(*this)[i], 2.0);
		}
		return lengthSquared;
	}
	[[nodiscard]] T GetLength() const
	{
		return sqrt(GetLengthSquared());
	}
	void Normalize()
	{
		T length = GetLength();
		// Avoid division with 0
		if (length != (T)0)
		{
			(*this) /= GetLength();
		}
	}
	[[nodiscard]] BasicVector GetNormalized()
	{
		BasicVector temporary = *this;
		temporary.Normalize();
		return temporary;
	}
	[[nodiscard]] T Dot(const BasicVector& other) const
	{
		T dot = (T)0;
		for (int i = 0; i < N; ++i)
		{
			dot += (*this)[i] * other[i];
		}
		return dot;
	}
	[[nodiscard]] BasicVector Cross(const BasicVector& other) const
	{
		static_assert(N == 3,
			"You only cross vectors of size 3");
		const Base& otherBase = other;
		return BasicVector(Base::y * otherBase.z - Base::z * otherBase.y,
						   Base::z * otherBase.x - Base::x * otherBase.z,
						   Base::x * otherBase.y - Base::y * otherBase.x);
	}

	BasicVector& operator+=(const BasicVector& other)
	{
		for (int i = 0; i < N; ++i)
		{
			(*this)[i] += other[i];
		}
		return *this;
	}
	BasicVector& operator-=(const BasicVector& other)
	{
		for (int i = 0; i < N; ++i)
		{
			(*this)[i] -= other[i];
		}
		return *this;
	}
	BasicVector& operator*=(const T value)
	{
		for (int i = 0; i < N; ++i)
		{
			(*this)[i] *= value;
		}
		return *this;
	}
	BasicVector& operator/=(const T value)
	{
		for (int i = 0; i < N; ++i)
		{
			(*this)[i] /= value;
		}
		return *this;
	}

	[[nodiscard]] BasicVector operator+(const BasicVector& other) const
	{
		BasicVector temporary = *this;
		temporary += other;
		return temporary;
	}
	[[nodiscard]] BasicVector operator-(const BasicVector& other) const
	{
		BasicVector temporary = *this;
		temporary -= other;
		return temporary;
	}
	[[nodiscard]] BasicVector operator*(const T value) const
	{
		BasicVector temporary = *this;
		temporary *= value;
		return temporary;
	}
	[[nodiscard]] BasicVector operator/(const T value) const
	{
		BasicVector temporary = *this;
		temporary /= value;
		return temporary;
	}
private:
	using Base = RawVector<T, N>;
};

template<class T, int N>
std::ostream& operator<<(std::ostream& ostream, const BasicVector<T, N>& vector)
{
	return ostream << vector.GetString();
}

template<class T>
using BasicVector2 = BasicVector<T, 2>;

template<class T>
using BasicVector3 = BasicVector<T, 3>;

template<class T>
using BasicVector4 = BasicVector<T, 4>;

using Vector2 = BasicVector2<float>;
using Vector3 = BasicVector3<float>;
using Vector4 = BasicVector4<float>;

using Vector2i = BasicVector2<int>;
using Vector3i = BasicVector3<int>;
using Vector4i = BasicVector4<int>;

namespace vector
{
	const inline Vector3 up(0.0f, 1.0f, 0.0f);
	const inline Vector3 down(0.0f, -1.0f, 0.0f);
	const inline Vector3 left(-1.0f, 0.0f, 0.0f);
	const inline Vector3 right(1.0f, 0.0f, 0.0f);
	const inline Vector3 backward(0.0f, 0.0f, 1.0f);
	const inline Vector3 forward(0.0f, 0.0f, -1.0f);
}