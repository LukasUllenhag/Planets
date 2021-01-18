#include "BenchmarkManager.h"
#include "BenchmarkEventFactory.h"
#include "../CustomException.h"
#include "../Console/ErrorLog.h"
#include "../Console/Log.h"
#include "../Console/ConsoleInput.h"
benchmark::Manager::Manager()
{
	// Make the file stream throw exceptions
	mFile.exceptions(std::ios::badbit | std::ios::failbit);
	OpenFile();
	mFile << "[";
}

benchmark::Manager& benchmark::Manager::Get()
{
 	static Manager instance;
	return instance;
}

void benchmark::Manager::BeginSession(const std::string& processName)
{
	std::lock_guard lockGuard(mMutex);

	assert(!SessionIsActive());

	const auto it = mSessionData.nameToId.find(processName);

	if (it == mSessionData.nameToId.end())
	{
		// This process does not yet exist

		CreateSession(processName);
	}

	mSessionData.activeId = mSessionData.nameToId[processName];
	mSessionData.isActive = true;
}

void benchmark::Manager::EndSession()
{
	std::lock_guard lockGuard(mMutex);

	assert(SessionIsActive());

	mSessionData.isActive = false;
}

void benchmark::Manager::Benchmark(const data::Timing& timingData)
{
	std::lock_guard lockGuard(mMutex);

	assert(SessionIsActive());

	ProcessEvent(
		EventFactory::CreateTiming(timingData, mSessionData.activeId)
	);
}

void benchmark::Manager::NameThread(const data::Thread& threadData)
{
	std::lock_guard lockGuard(mMutex);

	const auto it = mThreadIdToName.find(threadData.threadId);
	if (it == mThreadIdToName.end())
	{
		// This thread has not yet been named

		mThreadIdToName[threadData.threadId] = threadData.name;
		ProcessEvent(
			EventFactory::CreateThread(threadData, mSessionData.activeId)
		);
	}
	else
	{
		// Make sure we are not trying to rename the thread
		assert(mThreadIdToName[threadData.threadId] == threadData.name);
	}

}

void benchmark::Manager::SaveBenchmark()
{
	try
	{ 
		if (!UserWantsToSave())
		{
			return;
		}
		// Make sure that this is the only thread that logs or uses "CIN" during the duration of this scope.
		// We do not want the output, "LOG", to get detached from the input, "CIN".
		std::scoped_lock scopedLock(gLogMutex, gConsoleInputMutex);

		LOG("Save benchmark as: ");
		std::string name;
		CIN(name);

		const long long length = mFile.tellp();
		auto buffer = std::make_unique<char[]>(length);

		// Read the whole file into "buffer"
		mFile.seekg(std::ios::beg);
		mFile.read(buffer.get(), length);
		mFile.seekg(std::ios::end);

		std::ofstream saveFile; 
		// Make the file stream throw exceptions
		saveFile.exceptions(std::ios::badbit | std::ios::failbit);

		saveFile.open(SAVE_FILE_PATH + name + ".json");

		// Copy the current benchmarking content into the save file
		saveFile.write(buffer.get(), length);
	}
	catch (const std::exception& exception)
	{
		ERROR_LOG("Failed to save benchmark with message:" << std::endl << exception.what());
	}
}

bool benchmark::Manager::SessionIsActive() const
{
	return mSessionData.isActive;
}

void benchmark::Manager::CreateSession(const std::string& processName)
{
	ProcessEvent(
		EventFactory::CreateSession(benchmark::data::Session{ processName }, mSessionData.nextId)
	);
	mSessionData.nameToId[processName] = mSessionData.nextId++;
}

void benchmark::Manager::ProcessEvent(const benchmark::Event& event)
{
	try
	{
		mFile << event.GetData() + "," << std::endl;
	}
	catch (std::ios_base::failure)
	{
		ERROR_LOG("Failed to process event. Re-opening file");
		ReopenFile();
	}
}

void benchmark::Manager::OpenFile()
{
	try
	{ 
		// Open the file for both reading and writing. 
		// We want to discard the old content, hence "std::ios_base::trunc"
		mFile.open(FILE_PATH, std::ios_base::in | std::ios_base::out | 
							  std::ios_base::trunc | std::ios::binary);
	}
	catch (std::ios_base::failure)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open: " + FILE_PATH);
	}
}

void benchmark::Manager::ReopenFile()
{
	try
	{
		mFile.close();
	}
	catch (std::ios_base::failure)
	{
		// We do not rethrow the exception, since we still want to be able to re-open the file
		ERROR_LOG("Failed to close: " + FILE_PATH);
	}

	OpenFile();
}

bool benchmark::Manager::UserWantsToSave() const
{
	// Make sure that this is the only thread that logs or uses "CIN" during the duration of this scope.
	// We do not want the output, "LOG", to get detached from the input, "CIN".
	std::scoped_lock scopedLock(gLogMutex, gConsoleInputMutex);

	std::string input;

	// Loop until user answers question
	while (true)
	{
		// User inputs incorrect value, re-ask the question
		LOG("Do you want to the save the benchmark? Press \"y\" for yes and \"n\" for no: " << std::endl);

		CIN(input);
		if (input == "y" || input == "n")
		{
			break;
		}
	}
	return input == "y";
}
