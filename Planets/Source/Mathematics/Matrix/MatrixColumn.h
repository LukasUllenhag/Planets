#pragma once

template<class T, int N>
requires(std::is_arithmetic_v<T>)
class BasicMatrixColumn
{
public:
	template<class... Types>
	requires(sizeof...(Types) == N)
		BasicMatrixColumn(Types... values)
	{
		std::initializer_list<T> valueList{ values... };
		std::copy(valueList.begin(), valueList.end(), begin());
	}
	BasicMatrixColumn()
	{
		std::fill(begin(), end(), (T)0);
	}
	[[nodiscard]] T& operator[](const size_t index)
	{
		assert(index >= 0 && index < N);
		return mData[index];
	}
	[[nodiscard]] const T& operator[](const size_t index) const
	{
		return const_cast<BasicMatrixColumn&>(*this)[index];
	}

	[[nodiscard]] T* begin()
	{
		return mData;
	}
	[[nodiscard]] const T* begin() const
	{
		return mData;
	}
	[[nodiscard]] T* end()
	{
		return mData + N;
	}
	[[nodiscard]] const T* end() const
	{
		return mData + N;
	}
private:
	T mData[N];
};

template<class T>
using BasicMatrixColumn2 = BasicMatrixColumn<T, 2>;

template<class T>
using BasicMatrixColumn3 = BasicMatrixColumn<T, 3>;

template<class T>
using BasicMatrixColumn4 = BasicMatrixColumn<T, 4>;

using MatrixColumn2 = BasicMatrixColumn2<float>;
using MatrixColumn3 = BasicMatrixColumn3<float>;
using MatrixColumn4 = BasicMatrixColumn4<float>;