#pragma once

namespace benchmark
{
	namespace data
	{
		struct Thread
		{
			Thread(const std::string& name, unsigned int threadId);
			std::string name;
			unsigned int threadId = 0;
		};
	}
}