#Shader Vertex
#version 450 core

layout(location = 3) uniform uint width;
layout(location = 4) uniform float cellLength;

out VS_OUT
{
	vec2 uv;
}vsOut;

void main()
{
	vec3[4] vertices = { vec3(0.0, 0.0, 0.0), vec3(cellLength, 0.0, 0.0),
						 vec3(cellLength, 0.0, -cellLength), vec3(0.0, 0.0, -cellLength) };
	vec2[4] uvs = { vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0) };

	vec3 cellPosition =
		vec3(
			float(gl_InstanceID % width) * cellLength,
			0.0,
			-float(gl_InstanceID / width) * cellLength
		);

	gl_Position = vec4(vertices[gl_VertexID] + cellPosition, 1.0);
	vsOut.uv = uvs[gl_VertexID];
}

#Shader TessellationControl
#version 450 core

layout(vertices = 4) out;
layout(location = 0) uniform vec3 cameraPosition;
layout(location = 4) uniform float cellLength;

const float MINIMUM_DISTANCE = 10.0f;
const float MAXIMUM_DISTANCE = 100.0f;
const int HIGHEST_TESS_LEVEL = 10;
const int LOWEST_TESS_LEVEL = 1;
const float K = float(LOWEST_TESS_LEVEL - HIGHEST_TESS_LEVEL) / (MAXIMUM_DISTANCE - MINIMUM_DISTANCE);
const float M = HIGHEST_TESS_LEVEL - K * MINIMUM_DISTANCE;

in VS_OUT
{
	vec2 uv;
}tcIn[];

out TC_OUT
{
	vec2 uv;
}tcOut[];

void main()
{
	if (gl_InvocationID == 0)
	{
		vec3 center = gl_in[0].gl_Position.xyz + vec3(cellLength / 2.0, 0.0, -cellLength / 2.0);
		float distanceToCamera = length(cameraPosition - center);
		int level = int(distanceToCamera * K + M + 0.5);
		level = clamp(level, LOWEST_TESS_LEVEL, HIGHEST_TESS_LEVEL);

		// We set the tessellation levels to always be 10, since we have not yet
		// implemented support for seamless water
		level = 10;
		gl_TessLevelInner[0] = level;
		gl_TessLevelInner[1] = level;
		gl_TessLevelOuter[0] = level;
		gl_TessLevelOuter[1] = level;
		gl_TessLevelOuter[2] = level;
		gl_TessLevelOuter[3] = level;
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcOut[gl_InvocationID].uv = tcIn[gl_InvocationID].uv;
}

#Shader TessellationEvaluation
#version 450 core

layout(quads) in;
layout(fractional_even_spacing) in;

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 1) uniform mat4 rotationMatrix;
layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 5) uniform float[8] waterFactors;
layout(location = 13) uniform float time;

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

float GetWaterAltitude(const vec3 position)
{
	float perlinFrequency = waterFactors[0];
	float perlinAmplitude = waterFactors[1];
	float timeFactor = waterFactors[2];
	float frequencySinX = waterFactors[3];
	float frequencySinZ = waterFactors[4];
	float sinAmplitude = waterFactors[5];


	float perlin = PerlinNoise(vec3(position.x, 0.0, position.z) * perlinFrequency) * perlinAmplitude;

	float sinX = sin((position.x + perlin - time * timeFactor) * frequencySinX) * sinAmplitude;

	float sinZ = sin((position.z + perlin) * frequencySinZ) * sinAmplitude;

	return sinX + sinZ;
}

in TC_OUT
{
	vec2 uv;
}teIn[];

out TE_OUT
{
	noperspective vec3 position;
	vec2 uv;
}teOut;
void main()
{
	vec3 vertexPosition =
	mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
		gl_TessCoord.y).xyz;
	vertexPosition.y = GetWaterAltitude(vertexPosition);

	teOut.position = vertexPosition;
	gl_Position = projectionMatrix * rotationMatrix * vec4(vertexPosition - cameraPosition, 1.0);

	teOut.uv = mix(
		mix(teIn[0].uv, teIn[1].uv, gl_TessCoord.x),
		mix(teIn[3].uv, teIn[2].uv, gl_TessCoord.x),
		gl_TessCoord.y);
}

#Shader Fragment
#version 450 core

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 5) uniform float[8] waterFactors;
layout(location = 13) uniform float time;

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D normalMap;

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

float GetWaterAltitude(const vec3 position)
{
	float perlinFrequency = waterFactors[0];
	float perlinAmplitude = waterFactors[1];
	float timeFactor = waterFactors[2];
	float frequencySinX = waterFactors[3];
	float frequencySinZ = waterFactors[4];
	float sinAmplitude = waterFactors[5];


	float perlin = PerlinNoise(vec3(position.x, 0.0, position.z) * perlinFrequency) * perlinAmplitude;

	float sinX = sin((position.x + perlin - time * timeFactor) * frequencySinX) * sinAmplitude;

	float sinZ = sin((position.z + perlin) * frequencySinZ) * sinAmplitude;

	return sinX + sinZ;
}


const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));
const float DELTA = 0.0001;

in TE_OUT
{
	vec3 position;
	vec2 uv;
}fsIn;

out vec4 colour;

void main()
{
	// Calculate normal
	vec3 deltaX = vec3(fsIn.position.x + DELTA, 0.0, fsIn.position.z);
	deltaX.y = GetWaterAltitude(deltaX);

	vec3 deltaZ = vec3(fsIn.position.x, 0.0, fsIn.position.z - DELTA);
	deltaZ.y = GetWaterAltitude(deltaZ);

	vec3 normal = cross(deltaX, deltaZ);
	normal = normalize(normal);

	const vec3 forward = vec3(0.0, 0.0, 1.0);
	const vec3 tangent = normalize(cross(forward, normal));
	const vec3 bitangent = cross(normal, tangent);
	mat3 tnb;
	tnb[0] = tangent;
	tnb[1] = normal;
	tnb[2] = bitangent;
	normal = tnb * texture(normalMap, fsIn.uv).xyz;

	// Calculate specular lighting
	vec3 toCamera = cameraPosition - fsIn.position;
	toCamera = normalize(toCamera);
	vec3 reflectedRay = reflect(TO_SUN * -1.0, normal);
	float specular = dot(reflectedRay, toCamera);
	specular = max(specular, 0.0);
	specular = pow(specular, 10.0);

	// Calculate diffuse lighting
	float brightness = max(dot(normal, TO_SUN), 0.1);

	vec3 textureColour = texture(diffuseMap, fsIn.uv).rgb;
	vec3 blue = vec3(36.0 / 255.0, 74.0 / 255.0, 255.0 / 255.0);

	// We make the texture colour more blue
	vec3 mixedColour = mix(textureColour, blue, 0.8);

	colour = vec4(mixedColour * brightness + specular, 0.98);
}