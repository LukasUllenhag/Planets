#Shader Compute

#version 450 core

// Inside the method "RunTerrainGeneratorProgram" of class 
// "CelestialBody", we dispatch "nVertices" / 36 global work
// groups. The total amount of work groups will therefore be: 
// "local_size_x" * ("nVertices" / 36) = 12 * ("nVertices" / 36)
// = "nVertices" / 3, which is what we want, since each work group
// is responsible for updating three vertices.
layout(local_size_x = 12) in;

layout(location = 0) uniform int nCraters;
layout(location = 1) uniform float maxCraterTextureRadius;
layout(location = 2) uniform float craterFactors[21];

const int MAX_CRATER_COUNT = 1024;
const float PI = 3.1415926535;
const float MODEL_RADIUS = 1.0;

struct Vertex
{
	vec3 position;
	vec3 uv;
	vec3 normal;
};
layout(binding = 0, std430) coherent buffer Vertices
{
	Vertex vertices[];
};

struct CraterData
{
	vec3 position;
	float randomValue;
	bool hasTexture;
};

layout(binding = 2, std140) uniform CraterBuffer
{
	CraterData craterDatas[MAX_CRATER_COUNT];
}craterBuffer;

// vvv Perlin noise vvv
const int N_RANDOM_VALUES = 256;
layout(binding = 1, std140) uniform PermutationBuffer
{
	int permutationTable[N_RANDOM_VALUES * 2 - 1];
}permutationBuffer;

float Smoothstep(float t)
{
	return t * t * t * (10.0 + t * (6.0 * t - 15.0));
}
int AccessPermutationTable(int index)
{
	return permutationBuffer.permutationTable[index];
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

// Without the floor, "depth" would be the depth of
// the crater
const float DEPTH = craterFactors[3];

// The steepness of the cavity
const float STEEPNESS = craterFactors[4];

// The rim height as a share of the crater's radius
const float RIM_HEIGHT_SHARE = craterFactors[5];

// The distance from the crater's center
// to the highest point on the rim
const float RIM_POSITION = craterFactors[6];

// The amount of smoothing applied when combining
// the separate functions
const float SMOOTHNESS = craterFactors[7];

const float ROUGH_AMPLITUDE = craterFactors[8];
const float ROUGH_FREQUENCY = craterFactors[9];

const float FINE_AMPLITUDE = craterFactors[10];
const float FINE_FREQUENCY = craterFactors[11];

const float RIDGED_AMPLITUDE = craterFactors[12];
const float RIDGED_FREQUENCY = craterFactors[13];
const float RIDGED_OFFSET = craterFactors[14];

const float FRACTAL_FREQUENCY = craterFactors[15];
const float FRACTAL_AMPLITUDE = craterFactors[16];

const float MOUNTAIN_FREQUENCY = craterFactors[17];
const float MOUNTAIN_AMPLITUDE = craterFactors[18];

const float OCEAN_FLOOR_DEPTH = craterFactors[19];
const float OCEAN_DEPTH_MULTIPLIER = craterFactors[20];

// We calculate the factors: a and c, for the quadratic equation
// describing the shape of the cavity
const float CAVITY_FACTOR_A = STEEPNESS;
const float CAVITY_FACTOR_C = -DEPTH;

float SmoothMinimum(const float a, const float b, const float smoothness)
{
	// This function is based on the one found here:
	// https://iquilezles.org/www/articles/smin/smin.htm

	const float interpolationAmount = clamp((b - a) / smoothness * 0.5 + 0.5, 0.0, 1.0);
	return mix(b, a, interpolationAmount) -
		smoothness * interpolationAmount * (1.0 - interpolationAmount);
}
float SmoothMaximum(const float a, const float b, const float smoothness)
{
	// When "SmoothMinimum" gets a negative smooth factor, 
	// it will calculate the maximum instead of the minimum
	return SmoothMinimum(a, b, -smoothness);
}

// Takes in a value ("t") ranging from 0 to 1 and converts it to a
// value that is biased towards lower values, by the amount "amountOfBias"
float Bias(const float t, const float amountOfBias)
{
	return (t - amountOfBias * t) / (1.0 - amountOfBias * t);
}

float GetRandomFloorDepthShare(const float randomValue)
{
	const float lowestFloorDepthShare = 0.1;
	const float highestFloorDepthShare = 0.5;

	// Returns a random value ranging from "lowestFloorDepthShare" to 
	// "highestFloorDepthShare"
	return randomValue * (highestFloorDepthShare - lowestFloorDepthShare) 
		+ lowestFloorDepthShare;
}
float GetRandomCraterRadius(const float randomValue)
{	
	// Bias the random value towards lower values, to make
	// smaller craters more common
	const float biasedRandomValue = Bias(randomValue, 0.9);

	const float minRadius = 0.05;
	const float maxRadius = 0.2;

	// Returns a random value ranging from "minRadius" to 
	// "maxRadius"
	return biasedRandomValue * (maxRadius - minRadius) + minRadius;
}

// Returns the offset, caused by the crater, from the model's surface
float GetCraterOffset(const float distanceToCenter, const float craterRadius, const float randomValue)
{
	const float rimHeight = RIM_HEIGHT_SHARE * craterRadius;
	const float floorDepth = GetRandomFloorDepthShare(randomValue) * craterRadius;

	const float modifiedRimPosition = RIM_POSITION * craterRadius;

	// vvv Cavity offset calculation vvv
	const float cavityFactorB = (rimHeight -
		CAVITY_FACTOR_C - CAVITY_FACTOR_A * modifiedRimPosition * modifiedRimPosition)
		/ modifiedRimPosition;
	const float cavityOffset = CAVITY_FACTOR_A * distanceToCenter * distanceToCenter
		+ cavityFactorB * distanceToCenter + CAVITY_FACTOR_C;
	// ^^^ Cavity offset calculation ^^^


	// vvv Rim offset calculation vvv

	// We calculate the factor a for the quadratic equation
	// describing the shape of the rim (or more precisely, the outside
	// of the rim)
	const float offsettedRimPosition = craterRadius - modifiedRimPosition;
	const float rimCurveA = rimHeight /
		(offsettedRimPosition * offsettedRimPosition);

	const float offsettedDistanceToCenter = distanceToCenter - craterRadius;
	const float rimOffset = rimCurveA 
		* offsettedDistanceToCenter * offsettedDistanceToCenter;
	// ^^^ Rim offset calculation ^^^

	const float combinedOffset = SmoothMinimum(rimOffset, cavityOffset, SMOOTHNESS);
	return SmoothMaximum(combinedOffset, -floorDepth, SMOOTHNESS);
}

float GetRidgedPerlin(const vec3 position)
{
	if (abs(RIDGED_AMPLITUDE) < 0.0001)
	{
		// If the amplitude is really close to zero,
		// return zero and avoid any further calculations
		return 0.0;
	}

	// Calculate the ridged noise by taking the absolute value of the
	// perlin noise and then negating that result
	float perlinValue = (1.0 - 2.0 * abs((PerlinNoise(position * RIDGED_FREQUENCY) * 2.0 - 1.0)));
	perlinValue += RIDGED_OFFSET;
	perlinValue *= RIDGED_AMPLITUDE;

	// Apply some fractal noise, so that the
	// ridged noise will not look too smooth
	perlinValue += GetFractalPerlin(position, 3, 3.0, 0.1);

	// If the ridged amplitude is positive, "positiveAmplitudeFlag" will
	// be equal to 1 and if the ridged amplitude is negative (or zero), 
	// "positiveAmplitudeFlag" will be equal to -1
	const int positiveAmplitudeFlag = int(RIDGED_AMPLITUDE > 0.0) * 2 - 1;

	// Calculate the maximum, if the ridged amplitude is positive,
	// and the minimum if it is negative. To achieve this, without
	// conditional branching, we multiply the smootness by the 
	// "positiveAmplitudeFlag" (Note that if you negate
	// the smoothness factor for "SmoothMaximum" it will calculate
	// the minimum instead of the maximum).
	return SmoothMaximum(0.0, perlinValue, 
		SMOOTHNESS * float(positiveAmplitudeFlag));
}

float GetMountainOffset(const vec3 position)
{
	// Calculate the mountain offset by taking the absolute value of the
	// perlin noise and then negating that result
	float mountainOffset = (1.0 - abs(PerlinNoise(position * MOUNTAIN_FREQUENCY) * 2.0 - 1.0));

	// Push the mountains down so that only the highest part
	// of the mountain is visible
	mountainOffset -= 0.75;

	// Apply some fractal noise, so that the mountains will not 
	// look too smooth and remove the negative part of the offset
	mountainOffset = max(mountainOffset + GetFractalPerlin(position, 3, 3.0, 0.2), 0.0);
	mountainOffset *= MOUNTAIN_AMPLITUDE;

	// To limit the abundancy of the mountains, we
	// create a mountain mask 
	float mountainMask = PerlinNoise(position * 2.0) * 2.0 - 1.0;
	const float flatThreshold = 0.1;
	// Make the mask flat, if it is higher than "flatThreshold"
	// and lower than zero
	mountainMask = clamp(mountainMask, 0.0, flatThreshold);
	// Make the mask range from 0 to 1
	mountainMask /= flatThreshold;

	// Apply the mask
	mountainOffset *= mountainMask;

	return mountainOffset;
}
float GetTotalPerlinOffset(const vec3 position)
{
	const float roughOffset = 
		(PerlinNoise(position * ROUGH_FREQUENCY) * 2.0 - 1.0) * ROUGH_AMPLITUDE;

	const float fineOffset = 
		(PerlinNoise(position * FINE_FREQUENCY) * 2.0 - 1.0) * FINE_AMPLITUDE;
	
	const float fractalOffset = GetFractalPerlin(position, 3, FRACTAL_FREQUENCY, FRACTAL_AMPLITUDE);
	
	const float ridgedOffset = GetRidgedPerlin(position);

	float terrainOffset = roughOffset + fineOffset + fractalOffset + ridgedOffset;

	// If the terrain offset is negative, "terrainIsNegativeFlag" will
	// be equal to 1 and if the offset is positive (or zero), "terrainIsNegativeFlag" 
	// will be equal to 0
	const int terrainIsNegativeFlag = int(terrainOffset < 0.0);

	// If the terrain offset is negative, it is going to be multiplied by: 
	// 1 * ("OCEAN_DEPTH_MULTIPLIER" - 1) + 1 = "OCEAN_DEPTH_MULTIPLIER", and
	// if the terrain offset is positive (or zero) it is going to be multiplied
	// by: 0 * ("OCEAN_DEPTH_MULTIPLIER" - 1) + 1 = 1. In this way, we avoid 
	// conditional branching.
	terrainOffset *= float(terrainIsNegativeFlag) * (OCEAN_DEPTH_MULTIPLIER - 1) + 1.0;
	// Create some ocean floors
	terrainOffset = max(terrainOffset, -OCEAN_FLOOR_DEPTH);

	// Give the terrain some mountains
	terrainOffset += GetMountainOffset(position);

	return terrainOffset;
}


vec3 GetCraterUv(const vec3 vertexPosition, const vec3 craterPosition, const float imageRadius)
{
	const vec3 uAxis = normalize(
		cross(craterPosition, craterPosition + vec3(1.0, -1.0, 1.0)));
	// No need to normalize here, since "craterPosition" and "uAxis" have
	// a length of 1 and are perpendicular to each other
	const vec3 vAxis = cross(craterPosition, uAxis);

	// A vector that points from the crater's center to
	// the position of the vertex
	vec3 toVertex = vertexPosition - craterPosition;

	// Scale the vertex, so that a length of "imageRadius"
	// becomes a length of 1
	toVertex /= imageRadius;

	vec2 uv;
	// The x-coordinate is going to be
	// the distance that "toVertex" goes in
	// the direction of the u-axis
	uv.x = dot(toVertex, uAxis);

	// The y-coordinate is going to be
	// the distance that "toVertex" goes in
	// the direction of the v-axis
	uv.y = dot(toVertex, vAxis);
	
	// Make the Uv-coordinates 
	// range from 0 to 1 (instead
	// of -1 to 1)
	uv += 1.0;
	uv /= 2.0;

	// The last coordinate is set to 1,
	// to signal that the vertex should
	// be textured
	return vec3(uv, 1.0);
}

void main()
{
	// The index of the current work group
	const uint index = gl_GlobalInvocationID.x;

	// Each work group is responsible
	// for updating three vertices
	for (int i = 0; i < 3; ++i)
	{
		const Vertex vertex = vertices[index * 3 + i];

		// The total offest from the model's surface,
		// caused by all the craters
		float totalCraterOffset = 0.0;

		// Loop through all the craters, so that each crater
		// (if close enough) gets the chance to modify the vertex
		for (int j = 0; j < nCraters; ++j)
		{
			const vec3 craterPosition = craterBuffer.craterDatas[j].position;
			const float randomCraterValue = craterBuffer.craterDatas[j].randomValue;

			// The cosine of the angle between the vertex position and the
			// crater position, is equal to the dot product between the two
			// vectors, since they both have a length of 1. 
			const float cosine = dot(vertex.position, craterPosition);

			// The angle is simply the inverse cosine of the cosine. However,
			// due to floating-point errors, we have to make sure the cosine
			// is inside the correct range (-1 to 1), since the inverse cosine 
			// otherwise would be undefined.
			const float angle = acos(clamp(cosine, -1.0, 1.0));

			// The distance between two points on a sphere is equal to
			// the angle between the two points times the radius. Since the radius
			// of the model is 1, the distance between the points is simply 
			// equal to the angle.
			const float distance = angle;

			const float craterRadius = GetRandomCraterRadius(randomCraterValue);
			// Only proceed to calculate the UV-coordinates if
			// the crater should have a texture applied to it
			if(craterBuffer.craterDatas[j].hasTexture)
			{
				// The radius of the image is always three times
				// the radius of the crater, but not greater
				// than "maxCraterTextureRadius"
				const float imageRadius = min(craterRadius * 3.0, maxCraterTextureRadius);
				
				// Only proceed to calculate the UV-coordinates, if 
				// the distance between the crater and the vertex is
				// less than the radius of the image
				if (distance < imageRadius)
				{
					// Calculate the UV-coordinates
					vertices[index * 3 + i].uv = 
						GetCraterUv(vertex.position, craterPosition, imageRadius);
				}
			}
			// Only make the crater affect the position of
			// the vertex, if the distance between the crater
			// and the vertex is less than the radius of the
			// crater
			if (distance < craterRadius)
			{
				totalCraterOffset += GetCraterOffset(distance, craterRadius,
					randomCraterValue);
			}
		}

		// Make the length of the vertex position, the
		// radius of the model offsetted by all the craters
		// ("totalCraterOffset") and the perlin noise
		vertices[index * 3 + i].position = vertex.position 
			* (MODEL_RADIUS + totalCraterOffset + GetTotalPerlinOffset(vertex.position));
	}

	// Calculate the normal after we have set the positions
	// for all the vertices inside the triangle
	const vec3 right = vertices[index * 3 + 1].position - vertices[index * 3].position;
	const vec3 up = vertices[index * 3 + 2].position - vertices[index * 3].position;
	const vec3 normal = normalize(cross(right, up));
	for (int i = 0; i < 3; ++i)
	{
		// We use flat shading, i.e., all the triangle's 
		// vertices have the same normal
		vertices[index * 3 + i].normal = normal;
	}
}