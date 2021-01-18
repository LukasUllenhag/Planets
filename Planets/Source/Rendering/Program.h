#pragma once
#include "Shader.h"
#include <fstream>

class Program
{
public:
	Program(const std::string& filename);
	~Program();

	// One should not be able to copy a "Program" instance
	Program(const Program& other) = delete;
	Program& operator=(const Program& other) = delete;

	Program(Program&& other) noexcept;
	Program& operator=(Program&& other) noexcept;

	void Bind() const;
private:
	std::ifstream OpenFile(const std::string& filePath) const;
	std::vector<Shader> CreateShaders(std::ifstream& file, const std::string& filename);
	void HandleLinkError(const std::string& filename) const;
private:
	GLint mProgramName = 0;
	bool mContainsGlProgram = false;
	inline static const std::string FILE_PATH = "Source/Shaders/";
	inline static const std::string FILE_EXTENSION = ".shader";
};