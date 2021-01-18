#Shader Vertex
#version 450 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 1) uniform mat4 rotationMatrix;
layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 3) uniform float time;
layout(location = 4) uniform vec3 worldPosition;
layout(location = 5) uniform float scale;

// Perlin noise
layout(binding = 0) uniform usampler1D permutationTable;
const int N_RANDOM_VALUES = 256;

float Smoothstep(float t)
{
	return t * t * t * (10.0 + t * (6.0 * t - 15.0));
}
int AccessPermutationTable(int index)
{
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
// End of Perlin noise

out VS_OUT
{
	vec2 uv;
	float brightness;
	float specular;
}vsOut;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

void main()
{
	vec3 position = vertexPosition * scale + worldPosition;
	vsOut.uv = uv;

	// Diffuse lighting
	float brightness = dot(normal, TO_SUN);
	vsOut.brightness = max(brightness, 0.3);

	// Specular lighting
	vec3 toCamera = normalize(cameraPosition - position);
	vec3 reflectedRay = reflect(TO_SUN * -1.0, normal);
	float specular = dot(reflectedRay, toCamera);
	specular = max(specular, 0.0);
	specular = pow(specular, 20.0);
	vsOut.specular = specular;


	const float perlinPositionFrequency = 10.0;
	const float perlinAmplitude = 1.0 / 10.0;
	const float timeFactor = 10.0 / 17.0;
	const float scaledTime = time * timeFactor;

	vec3 distortedPosition = vertexPosition;
	distortedPosition.x += (PerlinNoise(
		vec3(
			distortedPosition.x * perlinPositionFrequency, 
			distortedPosition.z * perlinPositionFrequency,
			scaledTime)) * 2.0 - 1.0) * perlinAmplitude;

	distortedPosition.z += (PerlinNoise(
		vec3(
			distortedPosition.x * perlinPositionFrequency,
			distortedPosition.z * perlinPositionFrequency,
			scaledTime) + vec3(2.0, 2.0, 2.0)) * 2.0 - 1.0) * perlinAmplitude;
	
	gl_Position = projectionMatrix * rotationMatrix * vec4(distortedPosition * scale + worldPosition - cameraPosition, 1.0);
}

#Shader Fragment
#version 450 core

layout(location = 3) uniform float time;
layout(binding = 1) uniform sampler2D sampler;

// Perlin noise
layout(binding = 0) uniform usampler1D permutationTable;
const int N_RANDOM_VALUES = 256;

float Smoothstep(float t)
{
	return t * t * t * (10.0 + t * (6.0 * t - 15.0));
}
int AccessPermutationTable(int index)
{
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
// End of Perlin noise

in VS_OUT
{
	vec2 uv;
	float brightness;
	float specular;
} fsIn;

out vec4 colour;

void main()
{
	vec2 uv = fsIn.uv;

	const float perlinUvFrequency = 10.0;
	const float perlinAmplitude = 1.0 / 20.0;
	const float timeFactor = 10.0 / 17.0;
	const float perlinX = PerlinNoise(vec3(uv * perlinUvFrequency, time * timeFactor)) * perlinAmplitude;
	const float perlinY = PerlinNoise(vec3(uv * perlinUvFrequency, time * timeFactor) + vec3(2.0, 2.0, 2.0)) * perlinAmplitude;

	uv.x += perlinX;
	uv.y += perlinY;
	colour = texture(sampler, uv);
	colour.xyz *= fsIn.brightness;
	colour.xyz += fsIn.specular;
}
