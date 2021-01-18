#include "Shader.h"
#include <sstream>
#include "GlMacro.h"

Shader::Shader(std::string shaderSource, const std::string& filename)
{
	std::string typeAsString;
	mShaderName = GL(glCreateShader(GetType(shaderSource, typeAsString)));
	const char* const cString = shaderSource.c_str();

	GL(glShaderSource(mShaderName, 1, &cString, NULL));
	GL(glCompileShader(mShaderName));
	
	int successfullyCompiled = 0;
	GL(glGetShaderiv(mShaderName, GL_COMPILE_STATUS, &successfullyCompiled));

	if (!successfullyCompiled)
	{
		HandleCompileError(shaderSource, typeAsString, filename);
	}
	else
	{
		mContainsGlShader = true;
	}
}

Shader::~Shader()
{
	if(mContainsGlShader)
	{ 
		// Destructors should not throw exception, hence no GL macro
		glDeleteShader(mShaderName);
	}
}

Shader::Shader(Shader&& other) noexcept
{
	*this = std::forward<Shader&&>(other);
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	assert(this != &other);

	mShaderName = other.mShaderName;
	mContainsGlShader = other.mContainsGlShader;

	// Remove the resource from other
	other.mContainsGlShader = false;

	return *this;
}

GLuint Shader::GetShaderName() const
{
	return mShaderName;
}

void Shader::HandleCompileError(std::string shaderSource, const std::string& shaderType, const std::string& filename) const
{
	// "logLength" counts the null termination character
	int logLength = 0;
	GL(glGetShaderiv(mShaderName, GL_INFO_LOG_LENGTH, &logLength));

	std::string log;
	// Resize does not count the null termination character
	log.resize(logLength - 1);
	GL(glGetShaderInfoLog(mShaderName, logLength, NULL, log.data()));

	GL(glDeleteShader(mShaderName));
	AddLineNumbersToString(shaderSource);
	throw CREATE_CUSTOM_EXCEPTION("Failed to compile " + shaderType + 
		" shader in " + "\"" + filename + "\"" + "\n" + log + shaderSource);
}

void Shader::AddLineNumbersToString(std::string& string) const
{
	// The last line number will be equal to the amount of lines.
	const size_t lastLineNumber = std::count(string.begin(), string.end(), '\n') + 1;

	// The padding between the widest line number and the line that follows that line number
	const size_t widestLineNumberPadding = 2;

	// Every line number should have this width, so that the text gets properly aligned
	const size_t lineNumberWidth = std::to_string(lastLineNumber).size() + widestLineNumberPadding;

	std::istringstream inputStringStream(string);
	std::ostringstream stringStreamLineNumbers;
	unsigned int lineNumber = 1;
	while (inputStringStream.good())
	{
		std::string line;
		std::getline(inputStringStream, line);

		std::string lineNumberString = std::to_string(lineNumber++);
		// Every line number should have the same width, so that the text gets properly aligned
		lineNumberString.resize(lineNumberWidth, ' ');
		stringStreamLineNumbers << lineNumberString << line << "\n";
	}
	string = stringStreamLineNumbers.str();
}

GLenum Shader::GetType(std::string& shaderSource, std::string& typeAsString) const
{
	std::istringstream iStringStream(shaderSource);
	iStringStream >> typeAsString;
	// Remove the type identifier from the shader's source
	shaderSource = shaderSource.substr(iStringStream.tellg());
	auto iterator = msStringToType.find(typeAsString);
	if (iterator == msStringToType.end())
	{
		throw CREATE_CUSTOM_EXCEPTION("\"" + typeAsString + "\"" + " is an invalid shader type");
	}
	return iterator->second;
}
