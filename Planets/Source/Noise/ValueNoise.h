#pragma once

#include "../Mathematics/Vector/Vector.h"
#include "../Mathematics/Algorithms.h"
#include "RandomValueTable.h"
#include "PermutationTable.h"
#include "../CustomConcepts.h"

template<int N, int N_RANDOM_VALUES = 256>
// N_RANDOM_VALUES needs to be a power of two, since we need to use the &-operator instead
// of the %-operator
requires(IsPowerOfTwo(N_RANDOM_VALUES))
class ValueNoise
{
public:
	ValueNoise(const std::shared_ptr<RandomValueTable<N_RANDOM_VALUES>> randomValues, 
		const std::shared_ptr<PermutationTable<N_RANDOM_VALUES>> permutationTable)
		:
		mRandomValues(randomValues),
		mPermutationTable(permutationTable)
	{
		InitializeCornerOffets();
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

		// The amounts that we should interpolate between the random values with
		BasicVector<float, N> interpolationAmounts;
		std::transform(position.begin(), position.end(), location.begin(), interpolationAmounts.begin(),
			[this](float value, int flooredValue)
			{
				return Smoothstep(value - (float)flooredValue);
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

			cornerValues[i] = (*mRandomValues)[GetRandomIndex(cornerLocation)];
		}

		// Interpolate between the "cornerValues" based on the "interpolationAmounts"
		return Interpolate(cornerValues, interpolationAmounts);
	}
private:
	float Interpolate(float* cornerValues, const BasicVector<float, N>& interpolationAmounts) const
	{
		auto end = cornerValues + N_CORNERS;
		for (auto interpolationAmount : interpolationAmounts)
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
		return t * t * (3.0f - 2.0f * t);
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
private:
	static constexpr size_t N_CORNERS = Power(2, N);

	// Store the permutation table and the random values as shared pointers so that we do not have to
	// allocate a new permutation table and new random values for every instance of this class. Instances of
	// ValueNoise<2> and ValueNoise<3> can now for example share the same
	// permutation table and random values.
	std::shared_ptr<RandomValueTable<N_RANDOM_VALUES>> mRandomValues;
	std::shared_ptr<PermutationTable<N_RANDOM_VALUES>> mPermutationTable;

	std::vector<BasicVector<int, N>> mCornerOffsets;
};