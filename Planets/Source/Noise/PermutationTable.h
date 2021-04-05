#pragma once
#include <random>

template<class T, int N>
class BasicPermutationTable
{
public:
	BasicPermutationTable()
	{
		std::random_device randomNumberGenerator;
		std::mt19937 randomNumberEngine(randomNumberGenerator());
		std::uniform_int_distribution distributor(0, N - 1);

		std::generate(std::begin(mPermutationTable), std::end(mPermutationTable),
			[&]()
			{
				return (T)distributor(randomNumberEngine);
			});
	}
	T operator[](const size_t index) const
	{
		assert(index >= 0 && index < Size());
		return mPermutationTable[index];
	}
	const T* GetPointerToData() const
	{
		return mPermutationTable;
	}
	size_t Size() const
	{
		return N * 2 - 1;
	}
private:
	// Contains random indices that ranges from 0 to N - 1. The input into the 
	// table is the sum of two values that both range between 0 to N - 1 and therefore 
	// has a maximum value of N - 1 + N - 1 = N * 2 - 2. 
	// The table therefore needs a size of N * 2 - 1 (including the value 0).
	T mPermutationTable[N * 2 - 1];
};

template<int N>
using PermutationTable = BasicPermutationTable<unsigned char, N>;