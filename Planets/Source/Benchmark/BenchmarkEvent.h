#pragma once

namespace benchmark
{
	class EventFactory;

	// Can only be constructed through the "EventFactory" class
	class Event
	{
	public:
		const std::string& GetData() const;
	private:
		Event(std::initializer_list<std::pair<std::string, std::string>> attributesAndValues);

	private:
		friend class EventFactory;
		std::string mData;
	};
}