#pragma once
#include <fstream>
#include <mutex>
#include "Data/All.h"
#include "BenchmarkEvent.h"

namespace benchmark
{ 
	struct SessionData
	{
		std::unordered_map<std::string, int> nameToId;
		bool isActive = false;
		int activeId = 0;
		int nextId = 0; 
	};
	// Singleton
	class Manager
	{
	public:
		// Thread-safe
		static Manager& Get();
		// Thread-safe
		void BeginSession(const std::string& processName);
		// Thread-safe
		void EndSession();
		// Thread-safe
		void Benchmark(const data::Timing& timingData);
		// Thread-safe
		void NameThread(const data::Thread& threadData);

		void SaveBenchmark();
	private:
		Manager();
		void CreateSession(const std::string & processName);
		void ProcessEvent(const Event& event);
		bool SessionIsActive() const;
		void OpenFile();
		void ReopenFile();
		bool UserWantsToSave() const;
	private:
		std::fstream mFile;
		std::mutex mMutex;
		std::unordered_map<int, std::string> mThreadIdToName;
		SessionData mSessionData;
		inline static const std::string FILE_PATH = "Benchmarks/Benchmark.json";
		inline static const std::string SAVE_FILE_PATH = "Benchmarks/Saved/";
	};
}