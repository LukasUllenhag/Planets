#include "ThreadData.h"

benchmark::data::Thread::Thread(const std::string& name, unsigned int threadId)
	:
	name(name),
	threadId(threadId)
{
}
