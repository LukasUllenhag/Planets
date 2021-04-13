#Shader Vertex
#version 450 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec3 normal;

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 1) uniform mat4 viewRotation;
layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 3) uniform vec3 worldPosition;
layout(location = 4) uniform float scale;

out VS_OUT
{
	vec3 vertexNormal;
	vec3 toCamera;
	vec3 vertexPosition;
}vsOut;

void main()
{
	const vec3 position = worldPosition + vertexPosition * scale;
	vsOut.vertexNormal = normal;
	vsOut.toCamera = normalize(cameraPosition - position);
	vsOut.vertexPosition = vertexPosition;

	gl_Position = projectionMatrix * viewRotation * vec4(position - cameraPosition, 1.0);
}

#Shader Fragment
#version 450 core

layout(binding = 4) uniform sampler2D mountainNormalMap;

in VS_OUT
{
	vec3 vertexNormal;
	vec3 toCamera;
	vec3 vertexPosition;
} fsIn;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

out vec4 colour;

const vec3 COLOUR_LAYERS[3] = { 
	vec3(255.0 / 255.0, 237 / 255.0, 176 / 255.0), 
	vec3(158.0 / 255.0, 209.0 / 255.0, 63.0 / 255.0), 
	vec3(133.0 / 255.0, 123.0 / 255.0, 103.0 / 255.0)
};

const float steepnessBoundaries[3] = { 0.003, 0.02, 0.4 };
const float heightBoundaries[3] = { 1.0, 1.02, 1.059 };

// Returns a float whose integer part contains the colour layer index,
// and whose decimal part contains the amount we should interpolate between
// that index and the next index
float GetColourLayerIndex(const float parameter, const float interpolationDistanceShare, const float[3] boundaries)
{
	// Loop through all the boundaries, 
	// except the last one, since we can
	// not calculate "nextBoundary" for the
	// last boundary
	for (int i = 0; i < 2; ++i)
	{
		const float boundary = boundaries[i];
		const float nextBoundary = boundaries[i + 1];
		const float interpolationDistance = interpolationDistanceShare * (nextBoundary - boundary);

		// If the parameter is within the interpolationDistance,
		// interpolate between this index and the next index
		if (parameter <= boundary + interpolationDistance)
		{
			const float distanceToBoundary = parameter - boundary;
			// We clamp the interpolation amount between 0 and 0.999
			// instead of 0 to 1, since float(i) + 1.0 would, undesirably, 
			/// change the integer part of the float, i.e., the index
			const float interpolationAmount =
				clamp(distanceToBoundary / interpolationDistance, 0.0, 0.999);
			return float(i) + interpolationAmount;
		}
	}

	// Since the parameter is not within
	// the other boundaries, return a value
	// that will interpolate nearly all the way
	// from the second to last index to the 
	// last index. We can not return the last
	// index since that would cause an interpolation
	// between the last index and a non-existent index
	return 1.999;
}

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
// samples based on the normal) to sample from the normal map
vec3 GetTriplanarMappedMountainNormal(const vec3 texturePosition, const vec3 weights, const float normalStrength)
{
	const vec3 normalFrontAndBack = texture(mountainNormalMap, texturePosition.xy).xyz;
	const vec3 normalSides = texture(mountainNormalMap, texturePosition.yz).xyz;
	const vec3 normalTopAndBottom = texture(mountainNormalMap, texturePosition.zx).xyz;

	vec3 mapNormal = normalFrontAndBack * weights.z
		+ normalSides * weights.x
		+ normalTopAndBottom * weights.y;

	mapNormal.z = 1.0 - normalStrength;
	mapNormal = normalize(mapNormal);

	return mapNormal;
}

vec3 GetNormalMappedNormal(const vec3 vertexPosition, const vec3 faceNormal)
{
	const vec3 weights = GetTriplanarWeights(faceNormal, 8.0);
	const vec3 texturePosition = (vertexPosition + 1.0) / 2.0;

	const float amountOfNormalRepeats = 20.0;
	const vec3 mapNormal =
		GetTriplanarMappedMountainNormal(texturePosition * amountOfNormalRepeats, weights, 0.0);

	const vec3 tangent = normalize(cross(faceNormal, vec3(0.0, 1.0, 0.0)));
	// We do not have to normalize the binormal, since the vertex normal and 
	// the tangent both have a length of 1 and are perpendicular to each other
	const vec3 binormal = cross(faceNormal, tangent);

	// Construct the TBN matrix
	mat3 tbn;
	tbn[0] = tangent;
	tbn[1] = binormal;
	tbn[2] = faceNormal;

	return tbn * mapNormal;
}

void main()
{
	const vec3 localUp = normalize(fsIn.vertexPosition);
	const vec3 vertexNormal = normalize(fsIn.vertexNormal);

	// The dot product tells you how similar the 
	// direction of the normal and the direction of
	// "localUp" are. A high value will therefore
	// indicate that the surface is flat. To make 
	// a high value indicate the surface is steep,
	// we take the negative of the value.
	const float steepness = 1.0 - dot(vertexNormal, localUp);

	// Calculate the colour layer index based on the steepness of the surface
	const float steepnessColourLayerIndex = GetColourLayerIndex(steepness, 0.4, steepnessBoundaries);

	// Calculate the colour layer index based on the local height, i.e., the distance to the planet's origin
	const float heightColourLayerIndex = GetColourLayerIndex(length(fsIn.vertexPosition), 0.4, heightBoundaries);
	
	const int startIndexSteepness = int(steepnessColourLayerIndex);
	const int startIndexHeight = int(heightColourLayerIndex);

	const float interpolationAmountSteepness = steepnessColourLayerIndex - float(startIndexSteepness);
	const float interpolationAmountHeight = heightColourLayerIndex - float(startIndexHeight);
	
	const vec3 layerColourSteepness = mix(COLOUR_LAYERS[startIndexSteepness], COLOUR_LAYERS[startIndexSteepness + 1], interpolationAmountSteepness);
	const vec3 layerColourHeight = mix(COLOUR_LAYERS[startIndexHeight], COLOUR_LAYERS[startIndexHeight + 1], interpolationAmountHeight);
	
	// Make the colour equal to the average of the two layer colours
	colour = vec4(mix(layerColourSteepness, layerColourHeight, 0.5), 1.0);


	// vvv Normal calculation vvv

	const float blendDistance = 0.1;
	// Calculate how close the colour is to the mountain colour ("COLOUR_LAYERS[2]"), and
	// let that distance decide how much we should interpolate between the mountain normal
	// and the vertex normal
	const float normalInterpolationAmount = clamp(length(colour.xyz - COLOUR_LAYERS[2]) / blendDistance, 0.0, 1.0);
	
	const vec3 mountainNormal = GetNormalMappedNormal(fsIn.vertexPosition, vertexNormal);
	vec3 normal = mix(mountainNormal, vertexNormal, normalInterpolationAmount);
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

	// Apply the diffuse lighting
	colour.xyz *= brightness;
	// Apply the specular lighting
	colour.xyz += specular;
}
