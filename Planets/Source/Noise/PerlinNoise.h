#pragma once

#include "../Mathematics/Vector/Vector.h"
#include "../Mathematics/Algorithms.h"
#include "PermutationTable.h"
#include "../CustomConcepts.h"

// "VECTOR_SIZE" is the dimension of the diagonal pointing vectors. In order to not
// make the amount of diagonal vectors too few, we make sure that the value is at least 3.
template<int N, int N_RANDOM_VALUES = 256, int VECTOR_SIZE = std::max(3, N)>
// N_RANDOM_VALUES needs to be a power of two, since we need to use the &-operator instead
// of the %-operator
requires(IsPowerOfTwo(N_RANDOM_VALUES))
class PerlinNoise
{
public:
	PerlinNoise(const std::shared_ptr<PermutationTable<N_RANDOM_VALUES>> permutationTable)
		:
		mPermutationTable(permutationTable)
	{
		InitializeCornerOffets();
		InitializeDiagonalVectors();
	}

	float Get(const BasicVector<float, N>& position) const
	{
		// The floored integer position of the input vector 
		BasicVector<int, N> location;
		std::transform(position.begin(), position.end(), location.begin(),
			[](float value)
			{
				return (int)std::floor(value);
			});
		
		// A vector pointing from location to the input position
		BasicVector<float, N> toPosition;
		std::transform(position.begin(), position.end(), location.begin(), toPosition.begin(),
			[](float value, int flooredValue)
			{
				return value - (float)flooredValue;
			});
		
		// The same as "toPosition", except that we smoothstep all of the elements, in order
		// to make the interpolation between the random values smoother
		BasicVector<float, N> interpolationAmounts;
		std::transform(toPosition.begin(), toPosition.end(), interpolationAmounts.begin(),
			[this](float value)
			{
				return Smoothstep(value);
			});

		// Contains the random corner values
		float cornerValues[N_CORNERS];
		for (int i = 0; i < N_CORNERS; ++i)
		{
			// The location of the i-th corner
			BasicVector<int, N> cornerLocation = mCornerOffsets[i] + location;
			// We need to apply the modulo-operator, in order to not exceed the size
			// of the permutation table
			ApplyModulo(cornerLocation);

			BasicVector<float, N> cornerToPosition = toPosition - BasicVector<float, N>(mCornerOffsets[i]);
			// The random vectors have a size of "VECTOR_SIZE" and we need "cornerToPosition" to have the 
			// same size, in order for us to be able to perform the dot product
			BasicVector<float, VECTOR_SIZE> cornerToPositionSizeCorrected;
			std::copy(cornerToPosition.begin(), cornerToPosition.end(), cornerToPositionSizeCorrected.begin());

			cornerValues[i] = GetPerlinValue(GetRandomIndex(cornerLocation), cornerToPositionSizeCorrected);
		}

		// Interpolate between the "cornerValues" based on the "interpolationAmounts".
		// Interpolate returns a value between -1 and 1. We make the value range between 0
		// and 1 by first adding 1 to the value and then dividing the result by 2.
		return (Interpolate(cornerValues, interpolationAmounts) + 1.0f) / 2.0f;
	}
private:
	float GetPerlinValue(const size_t index, const BasicVector<float, VECTOR_SIZE>& cornerToPosition) const
	{
		// Make the index not exceed the size of the container by applying the %-operator
		return mDiagonalVectors[index % mDiagonalVectors.size()].Dot(cornerToPosition);
	}
	float Interpolate(float* cornerValues, const BasicVector<float, N>& interpolationAmounts) const
	{
		auto end = cornerValues + N_CORNERS;
		for(auto interpolationAmount : interpolationAmounts)
		{
			auto currentDestination = cornerValues;
			for (auto iterator = cornerValues; iterator != end; std::advance(iterator, 2), ++currentDestination)
			{
				// Interpolate between the adjacent corner values
				*currentDestination = Lerp(*iterator, *(iterator + 1), interpolationAmount);
			}
			// Set the end to be the iterator pointing to the result of the last interpolation + 1,
			// effectively cutting the amount of corner values in half after each "interpolationAmount"-loop
			end = currentDestination;
		}

		return cornerValues[0];
	}
	size_t GetRandomIndex(const BasicVector<int, N>& location) const
	{
		size_t index = 0;
		// Combine all of the location's elements into one index, effectively
		// hashing the location
		for (int i = 0; i < N; ++i)
		{
			index = (*mPermutationTable)[location[i] + index];
		}
	
		return index;
	}
	float Smoothstep(float t) const
	{
		return t * t * t * (10.0f + t * (6.0f * t - 15.0f));
	}
	void ApplyModulo(BasicVector<int, N>& location) const
	{
		// We apply the &-operator to the location, which is the same (as long as 
		// we subtract 1 from the value, e.g., "N_RANDOM_VALUES - 1") as 
		// applying the %-operator for positive values since "N_RANDOM_VALUES" is a
		// power of 2
		std::for_each(location.begin(), location.end(),
			[](int& value)
			{
				value = int(value & (N_RANDOM_VALUES - 1));
			});
	}
	void InitializeCornerOffets()
	{
		int moduloValues[N];
		for (int i = 0; i < N; ++i)
		{
			// The moduloValue decides how often the corresponding element
			// inside the diagonal vectors changes from 0 to 1. 
			// For i = 0, we get a modulo value of 2 ^ (0 + 1) = 2,
			// which means that the first (0) element is going to be 0 for 2 / 2 = 1 diagonalVectors
			// in a row. For i = 2, we get a modulo value of 2 ^ (2 + 1) = 8,
			// which means that the third (2) element is going to be 0 for 8 / 2 = 4 diagonalVectors
			// in a row, etc.
			moduloValues[i] = (int)std::pow(2.0, (double)i + 1);
		}

		for (int i = 0; i < N_CORNERS; ++i)
		{
			BasicVector<int, N> cornerOffset;
			for (int j = 0; j < N; ++j)
			{
				int moduloValue = moduloValues[j];
				// We can use a &-operator instead of the %-operator since moduloValue is
				// by definition a power of 2 (note that we then need to subtract 1 from
				// the modulo value: "moduloValue - 1"
				cornerOffset[j] = ((i & (moduloValue - 1)) < moduloValue / 2) ? 0 : 1;
			}
			mCornerOffsets.push_back(cornerOffset);
		}
	}
	void InitializeDiagonalVectors()
	{
		BasicVector<float, VECTOR_SIZE> diagonalVector;
		for (int i = 0; i < VECTOR_SIZE; ++i)
		{
			// Only one element of the diagonal vector is going to be 0, the i-th element. 
			// The rest of the elements are going to be either -1 or 1. The amount of
			// combinations for this iterations is therefore going to be the 2 ^ (amount of elements - 1)
			for (unsigned char combination = 0; combination < (int)std::pow(2.0, VECTOR_SIZE - 1); ++combination)
			{
				// Initialize all of the elements of the current diagonal vector
				for (int j = 0; j < VECTOR_SIZE; ++j)
				{
					if (j == i)
					{
						// The i-th element is always going to be 0
						diagonalVector[j] = 0.0f;
					}
					else
					{
						// We let the j-th bit of "combination" decide whether the j:th element should
						// contain -1 or 1. However, since we skip one bit when j = i, we need to subtract
						// 1 from j when j > i.
						int bit = j > i ? j - 1 : j;
						diagonalVector[j] = GetBit(combination, bit) ? -1.0f : 1.0f;
					}
				}
				mDiagonalVectors.push_back(diagonalVector);
			}
		}
	}
	bool GetBit(unsigned char number, int bitIndex) const
	{
		// Make sure that the "bitIndex" does not exceed the 
		// size of the number
		assert(bitIndex < 8);

		// Test if the bit at "bitIndex" is set
		return bool(number & (unsigned char)(0b1 << bitIndex));
	}
private:
	static constexpr size_t N_CORNERS = Power(2, N);

	// Store the permutation table as a shared pointer so that we do not have to
	// allocate a permutation table for every instance of this class. Instances of
	// PerlinNoise<2> and PerlinNoise<3> can now for example share the same
	// permutation table.
	std::shared_ptr<PermutationTable<N_RANDOM_VALUES>> mPermutationTable;

	std::vector<BasicVector<int, N>> mCornerOffsets;
	std::vector<BasicVector<float, VECTOR_SIZE>> mDiagonalVectors;
};