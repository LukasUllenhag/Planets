#include "Texture.h"
#include "PngLoader.h"
#include "GlMacro.h"

Texture::Texture(const std::string& filename)
{
	std::vector<unsigned char> buffer;
	lodepng::load_file(buffer, FILE_PATH + filename + FILE_EXTENSION);

	std::vector<unsigned char> image;
	unsigned int width;
	unsigned int height;
	lodepng::decode(image, width, height, buffer);

	// OpenGL begins to read the image in the lower left corner.
	// The first pixel from the image loaded is stored in the upper left corner.
	// We therefore need to invert the loaded image along the y-axis.
	RevertImage(image, width, height);

	GL(glCreateTextures(GL_TEXTURE_2D, 1, &mTextureName));
	GL(glTextureStorage2D(mTextureName, N_MIPMAP_LEVELS, GL_RGBA8, width, height));
	GL(glTextureSubImage2D(mTextureName, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &image.front()));
	GL(glGenerateTextureMipmap(mTextureName));
}

Texture::~Texture()
{
	// Destructors should not throw exception, hence no GL macro
	glDeleteTextures(1, &mTextureName);
}

void Texture::Bind(GLuint unit) const
{
	GL(glBindTextureUnit(unit, mTextureName));
}

void Texture::RevertImage(std::vector<unsigned char>& image, const int width, const int height) const
{
	// The width of the texture in bytes. Each pixel has a width of 4 bytes, 
	// hence we multiply the width, in pixels, by 4.
	const int byteWidth = width * 4;

	// Divide the image into a vector of rows
	std::vector<const unsigned char*> rows;
	rows.resize(height);
	std::generate(rows.begin(), rows.end(),
		[i = 0, &image, byteWidth]() mutable
	{
		return &image.front() + (i++) * byteWidth;
	});
	
	// Loop through the rows in reversed order
	std::for_each(rows.rbegin(), rows.rend(),
		[i = 0, &image, byteWidth](const unsigned char* row) mutable
	{
		// Copy each row back into the image, inverting the image along the y-axis
		std::copy_n(row, byteWidth, image.begin() + (i++) * byteWidth);
	});
}
