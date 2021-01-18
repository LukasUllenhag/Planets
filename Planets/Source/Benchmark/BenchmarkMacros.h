#pragma once
#include "BenchmarkTimer.h"
#include "BenchmarkSession.h"
#include "BenchmarkManager.h"

// Enables/disables all the benchmarking
#define ENABLE_BENCHMARKING 1
#define ALLOW_BENCHMARK_SAVING 1

#define CONCATENATE(a, b) CONCATENATE_I(a, b)
#define CONCATENATE_I(a, b) CONCATENATE_II(~, a ## b)
#define CONCATENATE_II(p, res) res

#if ENABLE_BENCHMARKING
// Enables benchmarking for the scope
#define BENCHMARK benchmark::Timer CONCATENATE(timer, __LINE__)(__FUNCSIG__)
#define NAMED_BENCHMARK(name) benchmark::Timer CONCATENATE(timer, __LINE__)(name)

// Turns the current scope into a session
#define CREATE_BENCHMARK_SESSION(name) benchmark::Session benchmarkSession(name)

// Names the current thread
#define NAME_THREAD(name) benchmark::Manager::Get().NameThread( \
benchmark::data::Thread{name, (unsigned int)std::hash<std::thread::id>()(std::this_thread::get_id())})

#if ALLOW_BENCHMARK_SAVING
// Saves the benchmark file
#define SAVE_BENCHMARK benchmark::Manager::Get().SaveBenchmark()
#else
#define SAVE_BENCHMARK
#endif

#else
#define BENCHMARK
#define NAMED_BENCHMARK
#define CREATE_BENCHMARK_SESSION(name)
#define NAME_THREAD(name)
#define SAVE_BENCHMARK
#endif