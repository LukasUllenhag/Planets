#include "../Rendering/Texture.h"
#include "../Noise/PerlinNoise.h"

class CelestialBodyTextures
{
public:
	CelestialBodyTextures(const std::string& craterTexture,
		const std::string& normalMap, const std::string& secondNormalMap);
	~CelestialBodyTextures();

	// Methods that bind the various textures to the wanted locations
	void BindTexture(GLuint location) const;
	void BindNormalMaps(GLuint location0, GLuint location1) const;
	void BindNormalInterpolation(GLuint location) const;
	void BindPermutationMap(GLuint location) const;
	void BindCraterTexture(GLuint location) const;
	void BindCraterSampler(GLuint location) const;
	void BindDefaultSampler(GLuint location) const;
private:
	void InitializePermutationMap(const PermutationTable<256>& permutationTable);
	void InitializeNormalInterpolation(const PerlinNoise<2>& perlinNoise);
	void InitializeTexture(const PerlinNoise<2>& perlinNoise);
	void InitializeCraterSampler();
	void InitializeDefaultSampler();

	float GetFractalPerlinNoise(const PerlinNoise<2>& perlinNoise, const Vector2& position,
		const int nOctaves, const float startFrequency, const float startAmplitude) const;
	float GetWarpedPerlinNoise(const PerlinNoise<2>& perlinNoise,
		const Vector2& position, const int nOctaves, const float frequency,
		const float amplitude, const float warpAmount) const;

	// Generates pixels, for the celestial body's surface 
	// texture, using a warped perlin noise
	std::unique_ptr<unsigned char[]> GetPixels(int width, int height,
		const PerlinNoise<2>& perlinNoise) const;

	// Generates pixels, for the normal interpolation texture, 
	// using a perlin noise
	std::unique_ptr<float[]> GetNormalInterpolationPixels(int width, int height,
		const PerlinNoise<2>& perlinNoise) const;
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

	// A texture that contains a permutation map that is
	// used for perlin noise calculations inside the shaders
	GLuint mPermutationMap = 0;

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
};