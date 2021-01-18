#pragma once
#include "CustomConcepts.h"
#include <chrono>

template<Clock C>
class BasicTimer
{
public:
	double Time()
	{
		const auto timePoint = C::now();
		const auto differenceDuration = timePoint - mPreviousTimePoint;
		mPreviousTimePoint = timePoint;

		const long long nanosecondsPassed = 
			(long long)std::chrono::duration_cast<std::chrono::nanoseconds>(differenceDuration).count();

		const double secondsPassed = (double)nanosecondsPassed / 1e+9;
		return secondsPassed;
	}
private:
	std::chrono::time_point<C> mPreviousTimePoint;
};

using Timer = BasicTimer<std::chrono::steady_clock>;

