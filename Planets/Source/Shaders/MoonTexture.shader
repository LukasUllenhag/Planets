#Shader Vertex
#version 450 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 1) uniform mat4 viewRotation;
layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 3) uniform vec3 worldPosition;
layout(location = 4) uniform float scale;

out VS_OUT
{
	vec3 uv;
	vec3 vertexNormal;
	vec3 toCamera;
	vec3 vertexPosition;
}vsOut;

void main()
{
	const vec3 position = worldPosition + vertexPosition * scale;
	vsOut.uv = uv;
	vsOut.vertexNormal = normal;
	vsOut.toCamera = normalize(cameraPosition - position);
	vsOut.vertexPosition = vertexPosition;

	gl_Position = projectionMatrix * viewRotation * vec4(position - cameraPosition, 1.0);
}

#Shader Fragment
#version 450 core

layout(binding = 1) uniform sampler2D surfaceTexture;
layout(binding = 2) uniform sampler2D craterTexture;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D secondNormalMap;
layout(binding = 5) uniform sampler2D normalInterpolationTexture;

// vvv Perlin noise vvv
layout(binding = 0) uniform usampler1D permutationTable;
const int N_RANDOM_VALUES = 256;

float Smoothstep(float t)
{
	return t * t * t * (10.0 + t * (6.0 * t - 15.0));
}
int AccessPermutationTable(int index)
{
	//return int(imageLoad(permutationTable, index).r);
	return int(texelFetch(permutationTable, index, 0).r);
}
uint GetRandomIndex(const ivec3 location)
{
	return AccessPermutationTable(AccessPermutationTable(AccessPermutationTable(location.x) + location.y) + location.z);
}
float GetRandomPerlinValue(const uint index, const vec3 toPosition)
{
	switch (index & 15)
	{
	case 0:
		return toPosition.x + toPosition.y;
	case 1:
		return toPosition.x - toPosition.y;
	case 2:
		return -toPosition.x + toPosition.y;
	case 3:
		return -toPosition.x - toPosition.y;
	case 4:
		return toPosition.y + toPosition.z;
	case 5:
		return toPosition.y - toPosition.z;
	case 6:
		return -toPosition.y + toPosition.z;
	case 7:
		return -toPosition.y - toPosition.z;
	case 8:
		return toPosition.x + toPosition.z;
	case 9:
		return toPosition.x - toPosition.z;
	case 10:
		return -toPosition.x + toPosition.z;
	case 11:
		return -toPosition.x - toPosition.z;
	case 12:
		return toPosition.x + toPosition.z;
	case 13:
		return toPosition.x - toPosition.z;
	case 14:
		return -toPosition.x + toPosition.z;
	case 15:
		return -toPosition.x - toPosition.z;
	default:
		return -1.0;
	}
}
float PerlinNoise(const vec3 position)
{
	int fx = int(floor(position.x));
	int fy = int(floor(position.y));
	int fz = int(floor(position.z));

	int x0 = int(fx & (N_RANDOM_VALUES - 1));
	int y0 = int(fy & (N_RANDOM_VALUES - 1));
	int z0 = int(fz & (N_RANDOM_VALUES - 1));

	int x1 = (x0 + 1) & (N_RANDOM_VALUES - 1);
	int y1 = (y0 + 1) & (N_RANDOM_VALUES - 1);
	int z1 = (z0 + 1) & (N_RANDOM_VALUES - 1);

	float tx = position.x - fx;
	float ty = position.y - fy;
	float tz = position.z - fz;

	float sx = Smoothstep(tx);
	float sy = Smoothstep(ty);
	float sz = Smoothstep(tz);

	float c000 = GetRandomPerlinValue(GetRandomIndex(ivec3(x0, y0, z0)), vec3(tx, ty, tz));
	float c100 = GetRandomPerlinValue(GetRandomIndex(ivec3(x1, y0, z0)), vec3(tx - 1, ty, tz));

	float c001 = GetRandomPerlinValue(GetRandomIndex(ivec3(x0, y0, z1)), vec3(tx, ty, tz - 1));
	float c101 = GetRandomPerlinValue(GetRandomIndex(ivec3(x1, y0, z1)), vec3(tx - 1, ty, tz - 1));

	float c010 = GetRandomPerlinValue(GetRandomIndex(ivec3(x0, y1, z0)), vec3(tx, ty - 1, tz));
	float c110 = GetRandomPerlinValue(GetRandomIndex(ivec3(x1, y1, z0)), vec3(tx - 1, ty - 1, tz));

	float c011 = GetRandomPerlinValue(GetRandomIndex(ivec3(x0, y1, z1)), vec3(tx, ty - 1, tz - 1));
	float c111 = GetRandomPerlinValue(GetRandomIndex(ivec3(x1, y1, z1)), vec3(tx - 1, ty - 1, tz - 1));

	float perlinValue = mix(
		mix(mix(c000, c100, sx), mix(c010, c110, sx), sy),
		mix(mix(c001, c101, sx), mix(c011, c111, sx), sy),
		sz
	);
	return (perlinValue + 1.0) / 2.0;
}
float GetFractalPerlin(const vec3 position, const int nOctaves,
	const float startFrequency)
{
	// The initial amplitude value
	// does not really matter, since
	// the final amplitude will get
	// scaled to 1
	float amplitude = 1.0;
	float frequency = startFrequency;
	float maxAmplitude = 0.0;
	float perlinValue = 0.0;

	// For each ocatave incrementation, we want to half the amplitude
	// and double the frequency
	for (int i = 0; i < nOctaves; i++, amplitude /= 2.0, frequency *= 2.0)
	{
		// "perlinNoise.Get" returns a value that ranges from 0 to 1. We 
		// therefore need to multiply by 2, and subtract by 1 to get a value
		// that ranges from -1 to 1. We then multiply by amplitude to get a 
		// value that ranges from -amplitude to amplitude.
		const float localPerlinValue = (PerlinNoise(position * frequency) * 2.0 - 1.0) * amplitude;
		perlinValue += localPerlinValue;

		// The max amplitude will increase by the amplitude
		// of the local perlin value
		maxAmplitude += amplitude;
	}

	// The perlin value ranges from -maxAmplitude to maxAmplitude. Adding
	// the max amplitude to the value and then dividing that sum by 2 * the 
	// max amplitude will make the perlin value range from 0 to 1.
	return (perlinValue + maxAmplitude) / (2.0 * maxAmplitude);
}
// For this overload of "GetFractalPerlin", one has the ability
// to specify the start amplitude
float GetFractalPerlin(const vec3 position, const int nOctaves,
	const float startFrequency, const float startAmplitude)
{
	float amplitude = startAmplitude;
	float frequency = startFrequency;
	float perlinValue = 0.0;

	// For each ocatave incrementation, we want to half the amplitude
	// and double the frequency
	for (int i = 0; i < nOctaves; i++, amplitude /= 2.0, frequency *= 2.0)
	{
		// "perlinNoise.Get" returns a value that ranges from 0 to 1. We 
		// therefore need to multiply by 2, and subtract by 1 to get a value
		// that ranges from -1 to 1. We then multiply by amplitude to get a 
		// value that ranges from -amplitude to amplitude.
		const float localPerlinValue = (PerlinNoise(position * frequency) * 2.0 - 1.0) * amplitude;
		perlinValue += localPerlinValue;
	}

	return perlinValue;
}
float GetWarpedPerlin(const vec3 position, const int nOctaves, 
	const float frequency, const float warpAmount)
{
	// Calculate the position offset using fractal noise
	vec3 offset =
	vec3(
		GetFractalPerlin(position,
		nOctaves, frequency) * 2.0 - 1.0,

		// The noise for the y-offset, is offsetted with (5.2, 1.3, 3.5), so that 
		// we will not get the same random value as the first coordinate
		GetFractalPerlin(position + vec3(5.2, 1.3, 3.5),
		nOctaves, frequency) * 2.0 - 1.0,

		// The noise for the y-offset, is offsetted with (9.2, 3.7, 5.2), so that 
		// we will not get the same random value as the second coordinate
		GetFractalPerlin(position + vec3(9.2, 3.7, 5.2),
		nOctaves, frequency) * 2.0 - 1.0
	);

	// Return a warped noise, i.e., a fractal noise that is 
	// offsetted with yet another fractal noise
	return GetFractalPerlin(position
		+ offset * warpAmount, nOctaves, frequency);
}
// ^^^ Perlin noise ^^^

// Get the weights, which decide how much each plane "weighs" for
// the triplanar mapping. "sharpness" controls how sharp the 
// transition should be between the three planes
vec3 GetTriplanarWeights(const vec3 normal, const float sharpness)
{
	vec3 weights = abs(normal);

	weights.x = pow(weights.x, sharpness);
	weights.y = pow(weights.y, sharpness);
	weights.z = pow(weights.z, sharpness);

	// Make the sum of the weights' coordinates = 1
	weights /= weights.x + weights.y + weights.z;

	return weights;
}


// Uses triplanar mapping (a technique, which results in seamless texturing, 
// where you sample the texture three times, and blend between the different 
// samples based on the normal) to sample from the surface texture
vec4 GetTriplanarMappedColour(const vec3 texturePosition, const vec3 weights)
{
	const vec4 textureFrontAndBack = texture(surfaceTexture, texturePosition.xy);
	const vec4 textureSides = texture(surfaceTexture, texturePosition.yz);
	const vec4 textureTopAndBottom = texture(surfaceTexture, texturePosition.zx);

	vec4 colour = textureFrontAndBack * weights.z
		+ textureSides * weights.x
		+ textureTopAndBottom * weights.y;

	return colour;
}

// Uses triplanar mapping (a technique, which results in seamless texturing, 
// where you sample the texture three times, and blend between the different 
// samples based on the normal) to sample from the normal map
vec3 GetTriplanarMappedNormal(const vec3 texturePosition, const vec3 weights, const float normalStrength)
{
	const vec3 normalFrontAndBack = texture(normalMap, texturePosition.xy).xyz;
	const vec3 normalSides = texture(normalMap, texturePosition.yz).xyz;
	const vec3 normalTopAndBottom = texture(normalMap, texturePosition.zx).xyz;

	vec3 mapNormal = normalFrontAndBack * weights.z
		+ normalSides * weights.x
		+ normalTopAndBottom * weights.y;

	mapNormal.z = 1.0 - normalStrength;
	mapNormal = normalize(mapNormal);

	return mapNormal;
}

// Uses triplanar mapping (a technique, which results in seamless texturing, 
// where you sample the texture three times, and blend between the different 
// samples based on the normal) to sample from the second normal map
vec3 GetTriplanarMappedSecondNormal(const vec3 texturePosition, const vec3 weights, const float normalStrength)
{
	const vec3 normalFrontAndBack = texture(secondNormalMap, texturePosition.xy).xyz;
	const vec3 normalSides = texture(secondNormalMap, texturePosition.yz).xyz;
	const vec3 normalTopAndBottom = texture(secondNormalMap, texturePosition.zx).xyz;

	vec3 mapNormal = normalFrontAndBack * weights.z
		+ normalSides * weights.x
		+ normalTopAndBottom * weights.y;

	mapNormal.z = 1.0 - normalStrength;
	mapNormal = normalize(mapNormal);

	return mapNormal;
}

// Uses triplanar mapping (a technique, which results in seamless texturing, 
// where you sample the texture three times, and blend between the different 
// samples based on the normal) to sample from the normal interpolation texture
float GetTriplanarMappedNormalInterpolation(const vec3 texturePosition, const vec3 weights)
{
	const float normalInterpolationFrontAndBack =
		texture(normalInterpolationTexture, texturePosition.xy).x;
	const float normalInterpolationSides =
		texture(normalInterpolationTexture, texturePosition.yz).x;
	const float normalInterpolationTopAndBottom =
		texture(normalInterpolationTexture, texturePosition.zx).x;

	const float normalInterpolation = normalInterpolationFrontAndBack * weights.z
		+ normalInterpolationSides * weights.x
		+ normalInterpolationTopAndBottom * weights.y;

	return normalInterpolation;
}

float GetWarpedColourAmount(const vec3 vertexPosition, 
	const float colourThreshold, const float blendDistance)
{
	// Make the warped perlin value from "GetWarpedPerlin" range from
	// -1 to 1, by first multiplying the value by 2, and then 
	// subtracting 1 from that product
	const float warpedPerlin =
		GetWarpedPerlin(vertexPosition, 3, 2.0, 1.2) * 2.0 - 1.0;

	// If "warpedPerlin" is higher than the sum of the colour threshold 
	// and the blend distance, we will get a value of 1. However, if 
	// "warpedPerlin" is only higher than the threshold by an amount
	// less than the blend distance, we will get a value between 0 and 1.
	// If "warpedPerlin" is less than the colourThreshold we will get
	// a value of 0. We use the smoothstep function to smooth out the
	// otherwise linear curve.
	return Smoothstep(clamp((warpedPerlin - colourThreshold) / blendDistance, 0.0, 1.0));
}

in VS_OUT
{
	vec3 uv;
	vec3 vertexNormal;
	vec3 toCamera;
	vec3 vertexPosition;
} fsIn;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

out vec4 colour;

void main()
{
	const vec3 vertexNormal = normalize(fsIn.vertexNormal);
	
	// vvv Triplanar sampling vvv
	const vec3 weights = GetTriplanarWeights(vertexNormal, 5.0);
	const vec3 texturePosition = (fsIn.vertexPosition + 1.0) / 2.0;
	
	const vec4 textureColour = GetTriplanarMappedColour(texturePosition, weights);

	const float normalStrength = 0.5;
	const vec3 mapNormal = GetTriplanarMappedNormal(texturePosition * 8.0, weights, normalStrength);
	const vec3 secondMapNormal = GetTriplanarMappedSecondNormal(texturePosition * 2.0, weights, normalStrength);
	
	const float normalInterpolation = GetTriplanarMappedNormalInterpolation(texturePosition, weights);
	// ^^^ Triplanar sampling ^^^

	// vvv Normal calculation vvv

	const vec3 tangent = normalize(cross(vec3(0.0, 1.0, 0.0), vertexNormal));
	// We do not have to normalize the binormal, since the vertex normal and 
	// the tangent both have a length of 1 and are perpendicular to each other
	const vec3 binormal = cross(vertexNormal, tangent);

	// Construct the TBN matrix
	mat3 tbn;
	tbn[0] = tangent;
	tbn[1] = binormal;
	tbn[2] = vertexNormal;

	// Calculate the two normals
	const vec3 firstNormal = tbn * mapNormal;
	const vec3 secondNormal = tbn * secondMapNormal;

	// Interpolate between the two normals
	vec3 normal = mix(firstNormal, secondNormal, normalInterpolation);

	// Normalize the interpolated normal
	normal = normalize(normal);

	// ^^^ Normal calculation ^^^

	// vvv Specular lighting vvv

	const vec3 toCamera = normalize(fsIn.toCamera);
	// The direction of the reflected sun ray
	const vec3 reflectedRay = reflect(TO_SUN * -1.0, normal);
	// If the reflected sun ray goes into the camera,
	// we want a high specularity
	float specular = dot(reflectedRay, toCamera);
	specular = max(specular, 0.0);
	specular = pow(specular, 10.0) * 0.01;

	// ^^^ Specular lighting ^^^

	// vvv Diffuse lighting vvv

	// If the normal is facing the sun, we 
	// want a high brightness
	float brightness = dot(normal, TO_SUN);
	brightness = max(brightness, 0.1);

	// ^^^ Diffuse lighting ^^^

	const vec4 warpedColour = vec4(0.35, 0.14, 0.14, 1.0);
	const float warpedColourAmount = GetWarpedColourAmount(fsIn.vertexPosition, -0.3, 0.5);
	// Interpolate between the texture colour and the warped colour, based
	// on "warpedColourAmount"
	colour = mix(textureColour, warpedColour, warpedColourAmount);

	// The last uv-coordinate is set to 1, if the vertex
	// should get textured. We only want to apply the texture
	// if all the vertices of the triangle have this coordinate 
	// set to 1, since we would like to avoid the artifacting that
	// occurs when one interpolates between a vertex that should
	// get textured and a vertex that should not get textured. Due
	// to floating-point inaccuracies, we check if the coordinate
	// is close to 1, instead of checking if it is equal to 1.
	const int applyCraterTextureFlag = int(fsIn.uv.z > 0.99);

	// When "applyCraterTextureFlag" is set to 1, we want to apply
	// the texture, and when the flag is set to 0 we do not want to
	// apply the texture. To achieve this result, we can simply multiply
	// the colour from the texture with the flag, and avoid conditional
	// branching.
	vec4 craterTextureColour = float(applyCraterTextureFlag) * texture(craterTexture, fsIn.uv.xy);

	// Interpolate between the colour and the texture colour, based on the texture
	// colour's alpha
	colour.rgb = mix(colour.rgb, craterTextureColour.rgb, craterTextureColour.a);
	
	// Apply the diffuse lighting
	colour.xyz *= brightness;
	// Apply the specular lighting
	colour.xyz += specular;
}
