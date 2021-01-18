#include "BenchmarkSession.h"
#include "BenchmarkManager.h"

benchmark::Session::Session(const std::string& name)
{
	Manager::Get().BeginSession(name);
}

benchmark::Session::~Session()
{
	Manager::Get().EndSession();
}