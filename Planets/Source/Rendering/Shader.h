#pragma once
#include "GL/glew.h"

class Shader
{
public:
	// We only need the file path in order to be able to notify the user
	// of which file the potential error occurred inside
	Shader(std::string shaderSource, const std::string& filename);
	~Shader();

	// One should not be able to copy a "Shader" instance
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	GLuint GetShaderName() const;
private:
	void HandleCompileError(std::string shaderSource, const std::string& shaderType, const std::string& filePath) const;
	void AddLineNumbersToString(std::string& string) const;
	GLenum GetType(std::string& shaderSource, std::string& typeAsString) const;
private:
	GLuint mShaderName = 0;
	bool mContainsGlShader = false;

	inline static const std::unordered_map<std::string, GLenum> msStringToType =
	{
		{"Vertex", GL_VERTEX_SHADER},
		{"Fragment", GL_FRAGMENT_SHADER},
		{"TessellationControl", GL_TESS_CONTROL_SHADER},
		{"TessellationEvaluation", GL_TESS_EVALUATION_SHADER},
		{"Geometry", GL_GEOMETRY_SHADER},
		{"Compute", GL_COMPUTE_SHADER}
	};
};