#include "CelestialBodyTextures.h"
#include "../Noise/PerlinNoise.h"
#include "../Rendering/GlMacro.h"
#include <fstream>

CelestialBodyTextures::CelestialBodyTextures(const std::string& craterTexture, 
	const std::string& normalMap, const std::string& secondNormalMap)
	:
	mCraterTexture(craterTexture),
	mNormalMap(normalMap),
	mSecondNormalMap(secondNormalMap)
{
	auto permutationTable = std::make_shared<PermutationTable<256>>();
	InitializeAllGlTextures(*permutationTable);
}

CelestialBodyTextures::CelestialBodyTextures(const std::string& craterTexture, 
	const std::string& normalMap, const std::string& secondNormalMap, 
	const celestialbodytextures::GenerateTexturesFlag&)
	:
	mCraterTexture(craterTexture),
	mNormalMap(normalMap),
	mSecondNormalMap(secondNormalMap)
{
	auto permutationTable = std::make_shared<PermutationTable<256>>();

	// Generate new data for the normal interpolation texture
	// and the surface texture, before initializing the textures
	// vvv
	PerlinNoise<2> perlinNoise(permutationTable);
	GenerateNormalInterpolationTexture(255, 255, perlinNoise);
	GenerateSurfaceTexture(255, 255, perlinNoise);
	// ^^^

	InitializeAllGlTextures(*permutationTable);
}

CelestialBodyTextures::~CelestialBodyTextures()
{
	// We do not want to throw an exception inside a destructor. 
	// Hence, we do not use the macro "GL".
	glDeleteTextures(1, &mTexture);
	glDeleteTextures(1, &mPermutationMap);
	glDeleteTextures(1, &mNormalInterpolationTexture);
	glDeleteSamplers(1, &mCraterSampler);
	glDeleteSamplers(1, &mDefaultSampler);
}

void CelestialBodyTextures::BindTexture(GLuint location) const
{
	GL(glBindTextureUnit(location, mTexture));
}

void CelestialBodyTextures::BindNormalMaps(GLuint location0, GLuint location1) const
{
	mNormalMap.Bind(location0);
	mSecondNormalMap.Bind(location1);
}

void CelestialBodyTextures::BindNormalInterpolation(GLuint location) const
{
	GL(glBindTextureUnit(location, mNormalInterpolationTexture));
}

void CelestialBodyTextures::BindPermutationMap(GLuint location) const
{
	GL(glBindTextureUnit(location, mPermutationMap));
}

void CelestialBodyTextures::BindCraterTexture(GLuint location) const
{
	mCraterTexture.Bind(location);
}

void CelestialBodyTextures::BindCraterSampler(GLuint location) const
{
	GL(glBindSampler(location, mCraterSampler));
}

void CelestialBodyTextures::BindDefaultSampler(GLuint location) const
{
	GL(glBindSampler(location, mDefaultSampler));
}

void CelestialBodyTextures::InitializePermutationMap(const PermutationTable<256>& permutationTable)
{
	// Pass in the permutation table as a one-dimensional texture
	GL(glCreateTextures(GL_TEXTURE_1D, 1, &mPermutationMap));
	GL(glTextureStorage1D(mPermutationMap, 1, GL_R8UI, (GLsizei)permutationTable.Size()));
	GL(glTextureSubImage1D(mPermutationMap, 0, 0, (GLsizei)permutationTable.Size(), GL_RED_INTEGER,
		GL_UNSIGNED_BYTE, permutationTable.GetPointerToData()));
}

void CelestialBodyTextures::InitializeNormalInterpolation()
{
	int width = 0;
	int height = 0;
	auto pixels = GetNormalInterpolationPixels(width, height);

	GL(glCreateTextures(GL_TEXTURE_2D, 1, &mNormalInterpolationTexture));
	GL(glTextureStorage2D(mNormalInterpolationTexture, 1, GL_R32F, width, height));
	GL(glTextureSubImage2D(mNormalInterpolationTexture, 0, 0, 0, width, height, GL_RED, GL_FLOAT, pixels.get()));
}

void CelestialBodyTextures::InitializeTexture()
{
	int width = 0;
	int height = 0;
	auto pixels = GetSurfacePixels(width, height);

	GL(glCreateTextures(GL_TEXTURE_2D, 1, &mTexture));
	GL(glTextureStorage2D(mTexture, 1, GL_RGBA8, width, height));
	GL(glTextureSubImage2D(mTexture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get()));
}

void CelestialBodyTextures::InitializeCraterSampler()
{
	GL(glCreateSamplers(1, &mCraterSampler));

	// If we, by accident, come outside the texture we want
	// the colour to be fully transparent
	const float colour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GL(glSamplerParameterfv(mCraterSampler, GL_TEXTURE_BORDER_COLOR, colour));

	GL(glSamplerParameteri(mCraterSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GL(glSamplerParameteri(mCraterSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
}

void CelestialBodyTextures::InitializeDefaultSampler()
{
	GL(glCreateSamplers(1, &mDefaultSampler));

	// The default behaviour for the texture wraps is "GL_REPEAT"
	GL(glSamplerParameteri(mDefaultSampler, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL(glSamplerParameteri(mDefaultSampler, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

void CelestialBodyTextures::InitializeAllGlTextures(const PermutationTable<256>& permutationTable)
{
	InitializePermutationMap(permutationTable);
	InitializeNormalInterpolation();
	InitializeTexture();
	InitializeCraterSampler();
	InitializeDefaultSampler();
}

float CelestialBodyTextures::GetFractalPerlinNoise(const PerlinNoise<2>& perlinNoise, 
	const Vector2& position, const int nOctaves, const float startFrequency, const float startAmplitude) const
{
	float frequency = startFrequency;
	float amplitude = startAmplitude;
	float maxAmplitude = 0.0f;
	float perlinValue = 0.0f;

	// For each ocatave incrementation, we want to half the amplitude
	// and double the frequency
	for (int i = 0; i < nOctaves; i++, amplitude /= 2.0f, frequency *= 2.0f)
	{
		// "perlinNoise.Get" returns a value that ranges from 0 to 1. We 
		// therefore need to multiply by 2, and subtract by 1 to get a value
		// that ranges from -1 to 1. We then multiply by amplitude to get a 
		// value that ranges from -amplitude to amplitude.
		const float localPerlinValue = (perlinNoise.Get(position * frequency) * 2.0f - 1.0f) * amplitude;
		perlinValue += localPerlinValue;

		// The max amplitude will increase by the amplitude
		// of the local perlin value
		maxAmplitude += amplitude;
	}

	// The perlin value ranges from -maxAmplitude to maxAmplitude. Adding
	// the max amplitude to the value and then dividing that sum by 2 * the 
	// max amplitude will make the perlin value range from 0 to 1.
	return (perlinValue + maxAmplitude) / (2.0f * maxAmplitude);
}

float CelestialBodyTextures::GetWarpedPerlinNoise(const PerlinNoise<2>& perlinNoise, 
	const Vector2& position, const int nOctaves, const float frequency,
	const float amplitude, const float warpAmount) const
{
	// Calculate the position offset using fractal noise
	Vector2 offset
	{
		GetFractalPerlinNoise(perlinNoise, position,
		nOctaves, frequency, amplitude),

		// The noise for the y-offset, is offsetted with (5.2, 1.3), so that 
		// we will not get the same random values for both coordinates
		GetFractalPerlinNoise(perlinNoise, position + Vector2(5.2f, 1.3f),
		nOctaves, frequency, amplitude)
	};

	// Return a warped noise, i.e., a fractal noise that is 
	// offsetted with yet another fractal noise
	return GetFractalPerlinNoise(perlinNoise, position
			+ offset * warpAmount, nOctaves, frequency, amplitude);
}

void CelestialBodyTextures::GenerateSurfaceTexture(const int width, const int height,
	const PerlinNoise<2>& perlinNoise) const
{
	// The size of a pixel, in bytes
	const int pixelSize = 4;
	auto pixels = std::make_unique<unsigned char[]>(width * height * pixelSize);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const float perlinValue = GetWarpedPerlinNoise(perlinNoise,
				Vector2((float)x, (float)y), 4, 0.01f, 1.0f, 1080.0f);

			const unsigned char grayscaleValue =
				unsigned char(perlinValue * 255.0f);

			// A pointer to the first byte of the current pixel
			unsigned char* const startOfPixel = &pixels[y * width * pixelSize + x * pixelSize];

			// The red, green, and blue channels (the first three bytes of the pixel)
			// should all have the same grayscale value
			std::fill_n(startOfPixel, 3, grayscaleValue);

			// The fourth byte of the pixel, the alpha channel, should
			// have the highest value, i.e., 255, since we want the colour
			// to be fully opaque
			startOfPixel[3] = 255;
		}
	}

	std::ofstream file;
	file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try
	{
		// We want to discard the old content, hence "std::ios_base::trunc"
		file.open(TEXTURE_PATH + SURFACE_TEXTURE_NAME + RAW_FILE_EXTENSION,
			std::ios_base::binary | std::ios_base::trunc);
		file << width << " " << height;
		file.write(reinterpret_cast<const char*>(pixels.get()), width * height * pixelSize);
	}
	catch (std::ios_base::failure)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open or write to file: "
			+ TEXTURE_PATH + SURFACE_TEXTURE_NAME + RAW_FILE_EXTENSION);
	}
}

std::unique_ptr<unsigned char[]> CelestialBodyTextures::GetSurfacePixels(int& width, int& height) const
{
	std::ifstream file;
	file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try
	{
		file.open(TEXTURE_PATH + SURFACE_TEXTURE_NAME + RAW_FILE_EXTENSION,
			std::ios_base::binary);

		file >> width;
		file >> height;

		// The size of a pixel, in bytes
		const int pixelSize = 4;
		auto pixels = std::make_unique<unsigned char[]>(width * height * pixelSize);
		file.read(reinterpret_cast<char*>(pixels.get()), width * height * pixelSize);

		return pixels;
	}
	catch (std::ios_base::failure)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open or read from file: "
			+ TEXTURE_PATH + SURFACE_TEXTURE_NAME + RAW_FILE_EXTENSION);
	}

	return std::unique_ptr<unsigned char[]>();
}

void CelestialBodyTextures::GenerateNormalInterpolationTexture(
	const int width, const int height, const PerlinNoise<2>& perlinNoise) const
{
	auto pixels = std::make_unique<float[]>(width * height);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			pixels[y * width + x] = perlinNoise.Get(Vector2((float)x, (float)y) * 0.1f);
		}
	}
	
	std::ofstream file;
	file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try
	{
		// We want to discard the old content, hence "std::ios_base::trunc"
		file.open(TEXTURE_PATH + NORMAL_INTERPOLATION_TEXTURE_NAME + RAW_FILE_EXTENSION, 
			std::ios_base::binary | std::ios_base::trunc);
		file << width << " " << height;
		file.write(reinterpret_cast<const char*>(pixels.get()), width * height * sizeof(float));
	}
	catch (std::ios_base::failure)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open or write to file: " 
			+ TEXTURE_PATH + NORMAL_INTERPOLATION_TEXTURE_NAME + RAW_FILE_EXTENSION);
	}
}

std::unique_ptr<float[]> CelestialBodyTextures::GetNormalInterpolationPixels(int& width, int& height) const
{
	std::ifstream file;
	file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	try
	{
		file.open(TEXTURE_PATH + NORMAL_INTERPOLATION_TEXTURE_NAME + RAW_FILE_EXTENSION, 
			std::ios_base::binary);

		file >> width;
		file >> height;

		auto pixels = std::make_unique<float[]>(width * height);
		file.read(reinterpret_cast<char*>(pixels.get()), width * height * sizeof(float));
		
		return pixels;
	}
	catch (std::ios_base::failure)
	{
		throw CREATE_CUSTOM_EXCEPTION("Failed to open or read from file: " 
			+ TEXTURE_PATH + NORMAL_INTERPOLATION_TEXTURE_NAME + RAW_FILE_EXTENSION);
	}

	return std::unique_ptr<float[]>();
}
