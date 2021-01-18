#pragma once
#include "../CustomException.h"

#ifdef DEBUG
#define GL(glFunctionCall) \
glFunctionCall; \
if(int errorCode = glGetError()) \
{ \
	std::string functionCallAsString(#glFunctionCall); \
	throw CREATE_CUSTOM_EXCEPTION(functionCallAsString + " failed with error code " + std::to_string(errorCode)); \
}
#else
#define GL(glFunctionCall) \
glFunctionCall;
#endif
