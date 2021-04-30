#include "CustomException.h"

CustomException::CustomException(const std::string& message, const std::string& filename, int lineNumber)
	:
	mFilename(filename),
	mLineNumber(lineNumber)
{
	mFullMessage =
		message +
		"\n\n" + "Filename: " + mFilename + "\n" +
		"Line number: " + std::to_string(mLineNumber);
}

const char* CustomException::what() const noexcept
{
	return mFullMessage.c_str();
}
