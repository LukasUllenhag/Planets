#pragma once
#include <chrono>
#include "BenchmarkManager.h"
#include <thread>
#include "../CustomConcepts.h"

namespace benchmark
{
	template<Clock C>
	class BasicTimer
	{
	public:
		BasicTimer(const std::string& name)
			:
			mName(name)
		{
			mStartTimePoint = C::now();
		}
		~BasicTimer()
		{
			const auto startDuration = 
				std::chrono::duration_cast<std::chrono::microseconds>(mStartTimePoint.time_since_epoch());
			const auto endDuration = 
				std::chrono::duration_cast<std::chrono::microseconds>(C::now().time_since_epoch());

			const long long duration = (endDuration - startDuration).count();
			const long long startTimepoint = startDuration.count();

			// We hash the thread id in order to convert it to an unsigned int
			const unsigned int hashedThreadId = (unsigned int)std::hash<std::thread::id>()(std::this_thread::get_id());
			benchmark::Manager::Get().Benchmark(
				benchmark::data::Timing{ mName, startTimepoint, duration, hashedThreadId });
		}
		// One should not be able to copy nor move a "BasicTimer" instance
		BasicTimer(const BasicTimer& other) = delete;
		BasicTimer& operator=(const BasicTimer& other) = delete;
	private:
		typename C::time_point mStartTimePoint;
		std::string mName;
	};

	using Timer = BasicTimer<std::chrono::high_resolution_clock>;
}