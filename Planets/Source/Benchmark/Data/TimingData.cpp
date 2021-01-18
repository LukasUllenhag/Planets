#include "TimingData.h"

benchmark::data::Timing::Timing(const std::string& name, long long timepoint, 
							    long long duration, unsigned int threadId)
	:
	name(name),
	timepoint(timepoint),
	duration(duration),
	threadId(threadId)
{
}
