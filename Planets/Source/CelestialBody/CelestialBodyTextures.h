#include "../Rendering/Texture.h"
#include "../Noise/PerlinNoise.h"


namespace celestialbodytextures
{
	struct GenerateTexturesFlag
	{};
	namespace constructionflag
	{
		// New noise textures will be generated, when
		// this flag is passed to the constructor of
		// "CelestialBodyTextures"
		const inline GenerateTexturesFlag generateTextures;
	}
}

class CelestialBodyTextures
{
public:
	CelestialBodyTextures(const std::string& craterTexture,
		const std::string& normalMap, const std::string& secondNormalMap);

	// The difference between this constructor and the one above, is that
	// this one generates new noise textures. The above constructor expects 
	// the noise textures to already have been generated.
	CelestialBodyTextures(const std::string& craterTexture,
		const std::string& normalMap, const std::string& secondNormalMap,
		const celestialbodytextures::GenerateTexturesFlag&);

	~CelestialBodyTextures();

	// Methods that bind the various textures to the wanted locations
	void BindTexture(GLuint location) const;
	void BindNormalMaps(GLuint location0, GLuint location1) const;
	void BindNormalInterpolation(GLuint location) const;
	void BindCraterTexture(GLuint location) const;
	void BindCraterSampler(GLuint location) const;
	void BindDefaultSampler(GLuint location) const;
private:
	void InitializeNormalInterpolation();
	void InitializeTexture();
	void InitializeCraterSampler();
	void InitializeDefaultSampler();
	void InitializeAllGlTextures(const PermutationTable<256>& permutationTable);

	float GetFractalPerlinNoise(const PerlinNoise<2>& perlinNoise, const Vector2& position,
		const int nOctaves, const float startFrequency, const float startAmplitude) const;
	float GetWarpedPerlinNoise(const PerlinNoise<2>& perlinNoise,
		const Vector2& position, const int nOctaves, const float frequency,
		const float amplitude, const float warpAmount) const;

	// Generates a raw surface texture for the celestial 
	// body, using a warped perlin noise, and writes it to
	// the texture folder
	void GenerateSurfaceTexture(int width, int height,
		const PerlinNoise<2>& perlinNoise) const;
	std::unique_ptr<unsigned char[]> GetSurfacePixels(int& width, int& height) const;

	// Generates a raw normal interpolation texture, 
	// using perlin noise, and writes it to the 
	// texture folder
	void GenerateNormalInterpolationTexture(int width, int height,
		const PerlinNoise<2>& perlinNoise) const;

	std::unique_ptr<float[]> GetNormalInterpolationPixels(int& width, int& height) const;
private:
	// A texture that could be applied to the
	// craters of the celestial body
	Texture mCraterTexture;
	
	// A normal map that should be applied
	// to the surface of the celestial body
	Texture mNormalMap;

	// A second normal map that should be applied
	// (in combination with the above normal map)
	// to the surface of the celestial body
	Texture mSecondNormalMap;

	// The surface texture for the celestial body
	GLuint mTexture = 0;

	// A texture, containing floats ranging from 0 to 1,
	// that decides how much we should interpolate between the
	// two normal maps
	GLuint mNormalInterpolationTexture = 0;

	// A sampler that should be bound in combination with the
	// the crater texture
	GLuint mCraterSampler = 0;

	// A sampler that should be bound when we no longer
	// want to use the crater sampler
	GLuint mDefaultSampler = 0;

	static const inline std::string TEXTURE_PATH = "Source/Textures/";
	static const inline std::string RAW_FILE_EXTENSION = ".raw";
	static const inline std::string NORMAL_INTERPOLATION_TEXTURE_NAME = "NormalInterpolation";
	static const inline std::string SURFACE_TEXTURE_NAME = "SurfaceTexture";
};