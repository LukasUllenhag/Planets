#pragma once
#include "GL/glew.h"

class Texture
{
public:
	Texture(const std::string& filename);
	~Texture();
	void Bind(GLuint location) const;
private:
	void RevertImage(std::vector<unsigned char>& image, int width, int height) const;
private:
	GLuint mTextureName;
	inline static const GLsizei N_MIPMAP_LEVELS = 4;
	inline static const std::string FILE_PATH = "Source/Textures/";
	inline static const std::string FILE_EXTENSION = ".png";
};