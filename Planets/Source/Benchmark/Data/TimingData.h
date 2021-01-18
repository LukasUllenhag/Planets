#pragma once

namespace benchmark
{
	namespace data
	{
		struct Timing
		{
			Timing(const std::string& name, long long timepoint, 
				   long long duration, unsigned int threadId);
			std::string name;
			long long timepoint = 0;
			long long duration = 0;
			unsigned int threadId = 0;
		};
	}
}

