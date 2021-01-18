#include "BenchmarkEventFactory.h"

benchmark::Event benchmark::EventFactory::CreateTiming(const data::Timing& data, unsigned int processId)
{

    return Event({ 
        { "name", "\"" + data.name + "\""}, {"cat", "\"Function\""}, {"ph", "\"X\""}, 
        {"ts", std::to_string(data.timepoint)}, {"dur", std::to_string(data.duration)},
        {"pid", std::to_string(processId)}, {"tid", std::to_string(data.threadId)}
        });
}

benchmark::Event benchmark::EventFactory::CreateSession(const data::Session& data, unsigned int processId)
{
    return Event({
        { "name", "\"process_name\""}, {"ph", "\"M\""},
        {"pid", std::to_string(processId)}, {"args", "{\"name\": \"" + data.name + "\"}"}
        });
}

benchmark::Event benchmark::EventFactory::CreateThread(const data::Thread& data, unsigned int processId)
{
    return Event({
        { "name", "\"thread_name\""}, {"ph", "\"M\""},
        {"pid", std::to_string(processId)}, {"args", "{\"name\": \"" + data.name + "\"}"}, 
        {"tid", std::to_string(data.threadId)}
        });
}