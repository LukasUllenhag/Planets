#include "Program.h"
#include "../CustomException.h"
#include <sstream>
#include "GlMacro.h"

Program::Program(const std::string& filename)
{
	const std::string wholeFilePath = FILE_PATH + filename + FILE_EXTENSION;
	std::ifstream file = OpenFile(wholeFilePath);
	std::vector<Shader> shaders = CreateShaders(file, filename);

	mProgramName = GL(glCreateProgram());

	for (const auto& shader : shaders)
	{
		GL(glAttachShader(mProgramName, shader.GetShaderName()));
	}

	GL(glLinkProgram(mProgramName));
	int successfullyLinked = 0;
	GL(glGetProgramiv(mProgramName, GL_LINK_STATUS, &successfullyLinked));

	if (!successfullyLinked)
	{
		HandleLinkError(filename);
	}
	else
	{
		mContainsGlProgram = true;
	}
}

Program::~Program()
{
	if (mContainsGlProgram)
	{
		// Destructors should not throw exception, hence no GL macro
		glDeleteProgram(mProgramName);
	}
}

Program::Program(Program&& other) noexcept
{
	*this = std::forward<Program&&>(other);
}

Program& Program::operator=(Program&& other) noexcept
{
	assert(this != &other);
	mProgramName = other.mProgramName;
	mContainsGlProgram = other.mContainsGlProgram;

	// Remove the resource from other
	other.mContainsGlProgram = false;

	return *this;
}

void Program::Bind() const
{
	assert(mContainsGlProgram);
	GL(glUseProgram(mProgramName));
}

std::ifstream Program::OpenFile(const std::string& filePath) const
{
	std::ifstream file;
	// Make the file stream throw exceptions
	file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	try
	{
		file.open(filePath);
	}
	catch (const std::ifstream::failure&)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open: " + filePath);
	}

	return file;
}

std::vector<Shader> Program::CreateShaders(std::ifstream& file, const std::string& filename)
{
	std::vector<Shader> shaders;

	std::string stringFile{ std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };
	const std::string startSignal = "#Shader";

	auto beginOfShaderSource = std::search(stringFile.begin(), stringFile.end(), startSignal.begin(), startSignal.end());
	while (beginOfShaderSource != stringFile.end())
	{
		// Add the size of the signal to get to the actual start of the shader's source
		beginOfShaderSource += startSignal.size();

		auto endOfShaderSource = std::search(beginOfShaderSource, stringFile.end(), startSignal.begin(), startSignal.end());
		shaders.emplace_back(std::string{ beginOfShaderSource, endOfShaderSource }, filename);

		// The beginning of the next shader's source is the end of this shader's source
		beginOfShaderSource = endOfShaderSource;
	}

	if (shaders.size() == 0)
	{
		throw CREATE_CUSTOM_EXCEPTION("\"" + filename + "\"" + " does not contain any shaders");
	}
	
	return shaders;
}

void Program::HandleLinkError(const std::string& filename) const
{
	// "logLength" counts the null termination character
	int logLength = 0;
	GL(glGetProgramiv(mProgramName, GL_INFO_LOG_LENGTH, &logLength));

	std::string log;
	// Resize does not count the null termination character
	log.resize(logLength -1);
	GL(glGetProgramInfoLog(mProgramName, logLength, NULL, log.data()));

	throw CREATE_CUSTOM_EXCEPTION("Failed to link program: \"" + filename + "\"" + "\n" + log);
}
