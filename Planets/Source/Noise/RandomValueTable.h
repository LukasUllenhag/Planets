#pragma once
#include <random>

template<int N>
class RandomValueTable
{
public:
	RandomValueTable()
	{
		std::random_device randomNumberGenerator;
		std::mt19937 randomNumberEngine(randomNumberGenerator());
		std::uniform_real_distribution distributor(0.0f, 1.0f);

		std::generate(std::begin(mRandomValues), std::end(mRandomValues),
			std::bind(distributor, std::ref(randomNumberEngine)));
	}
	float operator[](const size_t index) const
	{
		assert(index >= 0 && index < N);
		return mRandomValues[index];
	}
private:
	float mRandomValues[N];
};