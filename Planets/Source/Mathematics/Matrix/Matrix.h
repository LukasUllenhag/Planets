#pragma once
#include "../Algorithms.h"
#include "Source/Window/Window.h"
#include "MatrixColumn.h"

namespace matrix
{
	struct IdentityFlag
	{};
	namespace constructionflag
	{
		inline IdentityFlag identity;
	}
}


template<class T, int N>
requires(N >= 2 && std::is_arithmetic_v<T>)
class BasicMatrix
{
public:
	using Column = BasicMatrixColumn<T, N>;
	BasicMatrix(const std::initializer_list<Column>& columns)
	{
		std::copy(columns.begin(), columns.end(), this->mColumns);
	}
	BasicMatrix(const BasicMatrix& other)
	{
		std::copy(std::begin(other.mColumns), std::end(other.mColumns), std::begin(mColumns));
	}

	// Converting a smaller matrix into a bigger one, by extending
	// the matrix in an identity fashion
	template<int SmallN>
	explicit BasicMatrix(const BasicMatrix<T, SmallN>& smallerMatrix)
		requires(SmallN < N)
		:
		BasicMatrix(matrix::constructionflag::identity)
	{
		for (int i = 0; i < SmallN; ++i)
		{
			std::copy(smallerMatrix[i].begin(), smallerMatrix[i].end(), mColumns[i].begin());
		}
	}

	// Constructing an identity matrix
	BasicMatrix(matrix::IdentityFlag)
	{
		for (int i = 0; i < N; ++i)
		{
			(*this)[i][i] = (T)1;
		}
	}

	// The matrix gets filled with only zeros
	BasicMatrix() = default;

	[[nodiscard]] T* GetPointerToData()
	{
		return &(mColumns[0][0]);
	}
	[[nodiscard]] const T* GetPointerToData() const
	{
		return const_cast<BasicMatrix&>(*this).GetPointerToData();
	}

	[[nodiscard]] Column& operator[](const size_t index)
	{
		assert(index >= 0 && index < N);
		return mColumns[index];
	}
	[[nodiscard]] const Column& operator[](const size_t index) const
	{
		return const_cast<BasicMatrix&>(*this)[index];
	}

	[[nodiscard]] BasicMatrix operator*(const BasicMatrix& other) const
	{
		BasicMatrix temporary;
		for (int otherX = 0; otherX < N; ++otherX)
		{
			Column& column = temporary[otherX];
			for (int myY = 0; myY < N; ++myY)
			{
				T& value = column[myY];
				for (int myX = 0; myX < N; ++myX)
				{
					value += (*this)[myX][myY] * other[otherX][myX];
				}
			}
		}
		return temporary;
	}
	[[nodiscard]] BasicVector<T, N> operator*(const BasicVector<T, N>& vector) const
	{
		BasicVector<T, N> temporary;
		for (int y = 0; y < N; ++y)
		{
			T& value = temporary[y];
			for (int x = 0; x < N; ++x)
			{
				value += (*this)[x][y] * vector[x];
			}
		}
		return temporary;
	}

	BasicMatrix& operator*=(const BasicMatrix& other)
	{
		*this = (*this) * other;
		return *this;
	}
private:
	Column mColumns[N];
};

template<class T, int N>
std::vector<std::string> GetColumnAsString(const BasicMatrixColumn<T,N>& column, const size_t decimalCount)
{
	std::vector<std::string> stringColumn;

	// Convert the values from the column into strings
	std::transform(column.begin(), column.end(), std::back_inserter(stringColumn),
		[decimalCount](T value)
		{
			return ConvertFloatingPointToRoundedString(value, decimalCount);
		});

	// Get the string that contains the most amount of characters
	const std::string widestString = *std::max_element(stringColumn.begin(), stringColumn.end(),
		[](const std::string& a, const std::string& b)
		{
			return a.size() < b.size();
		});
	const size_t maxStringWidth = widestString.size();

	// Make all the strings inside the column the same width,
	// for alignment purposes
	std::for_each(stringColumn.begin(), stringColumn.end(),
		[maxStringWidth](std::string& string)
		{
			string.resize(maxStringWidth, ' ');
		});

	return stringColumn;
}

template<class T, int N>
std::ostream& operator<<(std::ostream& ostream, const BasicMatrix<T, N>& matrix)
{
	using Column = BasicVector<T, N>;
	using StringColumn = std::vector<std::string>;

	const size_t decimalCount = 2;
	std::vector<StringColumn> stringColumns;
	for (int x = 0; x < N; ++x)
	{
		stringColumns.push_back(GetColumnAsString(matrix[x], decimalCount));
	}

	for (int y = 0; y < N; ++y)
	{
		ostream << "|";
		for (int x = 0; x < N - 1; ++x)
		{
			ostream << stringColumns[x][y] << "  ";
		}
		ostream << stringColumns[N - 1][y] << "|" << std::endl;
	}

	return ostream;
}

template<class T>
using BasicMatrix2 = BasicMatrix<T, 2>;

template<class T>
using BasicMatrix3 = BasicMatrix<T, 3>;

template<class T>
using BasicMatrix4 = BasicMatrix<T, 4>;

using Matrix2 = BasicMatrix2<float>;
using Matrix3 = BasicMatrix3<float>;
using Matrix4 = BasicMatrix4<float>;

namespace matrix
{
	// Return the a rotationMatrix that rotates a vector "radians" radians
	// counterclockwise around the x-axis
	template<class T>
	BasicMatrix3<T> GetRotationX(const T radians)
	{
		return BasicMatrix3<T>({ BasicMatrixColumn3<T>((T)1, (T)0, (T)0),
								 BasicMatrixColumn3<T>((T)0, cos(radians), sin(radians)),
								 BasicMatrixColumn3<T>((T)0, -sin(radians), cos(radians)) });
	}
	// Return the a rotationMatrix that rotates a vector "radians" radians
	// counterclockwise around the y-axis
	template<class T>
	BasicMatrix3<T> GetRotationY(const T radians)
	{
		return BasicMatrix3<T>({ BasicMatrixColumn3<T>((T)cos(radians), (T)0, -sin(radians)),
								 BasicMatrixColumn3<T>((T)0, (T)1, (T)0),
								 BasicMatrixColumn3<T>((T)sin(radians), (T)0, cos(radians)) });
	}
	// Return the a rotationMatrix that rotates a vector "radians" radians
	// counterclockwise around the z-axis
	template<class T>
	BasicMatrix3<T> GetRotationZ(const T radians)
	{
		return BasicMatrix3<T>({ BasicMatrixColumn3<T>((T)cos(radians), -(T)sin(radians), (T)0),
								 BasicMatrixColumn3<T>((T)sin(radians), (T)cos(radians), (T)0),
								 BasicMatrixColumn3<T>((T)0, (T)0, (T)1) });
	}
	
	template<class T>
	BasicMatrix3<T> GetRotation(const T radiansX, const T radiansY, const T radiansZ)
	{
		// Rotate first around the y-axis, then around the x-axis and lastly around the z-axis
		return GetRotationZ(radiansZ) * GetRotationX(radiansX) * GetRotationY(radiansY);
	}
	
	template<class T>
	BasicMatrix4<T> GetProjection(const T left, const T right,
										const T top, const T bottom,
										const T near, const T far)
	{
		return BasicMatrix4<T>({ BasicMatrixColumn4<T>((T)2 * near / (right - left), (T)0, (T)0, (T)0),
								 BasicMatrixColumn4<T>((T)0, (T)2 * near / (top - bottom), (T)0, (T)0),
								 BasicMatrixColumn4<T>((right + left) / (right - left), (top + bottom) / (top - bottom),
									(near + far) / (near - far), (T)-1),
								 BasicMatrixColumn4<T>((T)0, (T)0, (T)2 * far * near / (near - far), (T)0) });
	}

	template<class T>
	BasicMatrix4<T> GetProjection(const T fovY, const T near, const T far)
	{
		T top = near * tan(fovY / (T)2);
		T bottom = -top;
		T aspectRatio = (T)Window::GetWidth() / (T)Window::GetHeight();
		T right = aspectRatio * top;
		T left = -right;
		return GetProjection(left, right, top, bottom, near, far);
	}
}