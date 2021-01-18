#pragma once

namespace benchmark
{
	class Session
	{
	public:
		Session(const std::string& name);
		~Session();
		// One should not be able to copy nor move a "Session" instance
		Session(const Session& other) = delete;
		Session& operator=(const Session& other) = delete;
	};
}