#pragma once
#include "BenchmarkEvent.h"
#include "Data/All.h"

namespace benchmark
{
	class EventFactory
	{
	public:
		static Event CreateTiming(const data::Timing& data, unsigned int processId);
		static Event CreateSession(const data::Session& data, unsigned int processId);
		static Event CreateThread(const data::Thread& data, unsigned int processId);
	};
}