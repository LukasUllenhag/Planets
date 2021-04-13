#Shader Vertex
#version 450 core

out vec2 uv;

void main()
{
	vec3[4] vertexPositions = { vec3(1.0, -1.0, 0.0), vec3(1.0, 1.0, 0.0), vec3(-1.0, -1.0, 0.0), vec3(-1.0, 1.0, 0.0) };
	vec2[4] uvs = { vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 0.0), vec2(0.0, 1.0) };

	uv = uvs[gl_VertexID];
	gl_Position = vec4(vertexPositions[gl_VertexID], 1.0);
}

#Shader Fragment
#version 450 core

layout(binding = 0) uniform sampler2D renderedSceneTexture;
layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 2) uniform sampler2D waterNormalMap;

layout(location = 0) uniform float near;
layout(location = 1) uniform float far;
layout(location = 2) uniform float left;
layout(location = 3) uniform float right;
layout(location = 4) uniform float top;
layout(location = 5) uniform float bottom;

layout(location = 6) uniform float planetRadius;
// The position of the planet rotated by the 
// view rotation, since most calculations done in
// the shader occur inside that space
layout(location = 7) uniform vec3 planetPosition;

layout(location = 8) uniform mat4 viewRotation;
layout(location = 9) uniform mat4 viewRotationInverse;

layout(location = 10) uniform mat3 normalMapRotation0;
layout(location = 11) uniform mat3 normalMapRotation1;

// The "TO_SUN"-vector rotated by the 
// view rotation, since most calculations done in
// the shader occur inside that space
const vec3 TO_SUN = (viewRotation * vec4(normalize(vec3(1.0, 5.0, 0.0)), 1.0)).xyz;

struct QuadraticSolutions
{
	float first;
	float second;
};

QuadraticSolutions SolveQuadraticEquation(const float p, const float q)
{
	const float firstPart = -p / 2.0;
	const float valueToSquareRoot = pow(p / 2.0f, 2.0f) - q;
	if (valueToSquareRoot < 0.0f)
	{
		// Both answers are imaginary
		return QuadraticSolutions(-1.0, -1.0);
	}
	else if (valueToSquareRoot == 0.0f)
	{
		// Both answers are equal to "firstPart"
		return QuadraticSolutions(firstPart, firstPart);
	}

	const float squareRoot = sqrt(valueToSquareRoot);

	return QuadraticSolutions(firstPart + squareRoot, firstPart - squareRoot);
}

struct SphereIntersections
{
	float first;
	float second;
};

// Returns the distances one has to travel along "direction" in order
// to intersect the sphere. A negative distance means that one has to 
// travel in the opposite direction of "direction" to be able to intersect
// the sphere.
SphereIntersections GetSphereIntersections(const vec3 direction, const vec3 spherePosition,
	const float radius)
{
	const float p = -dot(direction, spherePosition) * 2.0;
	const float spherePositionLength = length(spherePosition);
	const float q = spherePositionLength * spherePositionLength - radius * radius;

	QuadraticSolutions quadraticSolutions = SolveQuadraticEquation(p, q);
	return SphereIntersections(quadraticSolutions.first, quadraticSolutions.second);
}

// Sorts the sphere intersections so that the first value
// becomes the distance to the closest visible sphere intersection
SphereIntersections SortSphereIntersections(SphereIntersections sphereIntersections)
{
	float distanceToClosestVisibleIntersection = 0.0;
	float distanceToOtherIntersection = 0.0;

	// We could rewrite this without conditional branching, but
	// it would be more difficult to understand
	if (sphereIntersections.first > 0.0 && sphereIntersections.second > 0.0)
	{
		// If both distances are positive, it means that both intersections are
		// visible. We therefore want the intersection that has the lowest disance
		// (is the closest to the camera).
		distanceToClosestVisibleIntersection = min(sphereIntersections.first, sphereIntersections.second);
		distanceToOtherIntersection = max(sphereIntersections.first, sphereIntersections.second);
	}
	else
	{
		// If at least one distance is negative, we want the one that is positive (visible), i.e.,
		// the maximum of the two
		distanceToClosestVisibleIntersection = max(sphereIntersections.first, sphereIntersections.second);
		distanceToOtherIntersection = min(sphereIntersections.first, sphereIntersections.second);
	}
	return SphereIntersections(distanceToClosestVisibleIntersection, distanceToOtherIntersection);
}

vec3 ConvertPixelPositionToWorldPosition(const vec3 pixelScreenPosition)
{
	vec3 worldPosition = vec3(0.0);
	worldPosition.z =
		(-2.0 * far * near) / ((pixelScreenPosition.z + (near + far) / (near - far)) * (near - far));
	worldPosition.x =
		(pixelScreenPosition.x - (left + right) / (left - right)) * (left - right) * worldPosition.z / (2.0 * near);
	worldPosition.y =
		(pixelScreenPosition.y - (bottom + top) / (bottom - top)) * (bottom - top) * worldPosition.z / (2.0 * near);

	return worldPosition;
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
vec3 GetTriplanarMappedWaterNormal(const vec3 texturePosition, const vec3 weights, const float normalStrength)
{
	const vec3 normalFrontAndBack = texture(waterNormalMap, texturePosition.xy).xyz;
	const vec3 normalSides = texture(waterNormalMap, texturePosition.yz).xyz;
	const vec3 normalTopAndBottom = texture(waterNormalMap, texturePosition.zx).xyz;

	vec3 mapNormal = normalFrontAndBack * weights.z
		+ normalSides * weights.x
		+ normalTopAndBottom * weights.y;

	mapNormal.z = 1.0 - normalStrength;
	mapNormal = normalize(mapNormal);

	return mapNormal;
}
vec3 GetNormalMappedNormal(const vec3 toPointOnSphere, const vec3 faceNormal)
{
	const vec3 weights = GetTriplanarWeights(faceNormal, 8.0);
	const vec3 texturePosition = (toPointOnSphere + 1.0) / 2.0;

	const float amountOfNormalRepeats = 20.0;
	const vec3 mapNormal = 
		GetTriplanarMappedWaterNormal(texturePosition * amountOfNormalRepeats, weights, 0.0);

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

vec3 GetWaterColour(const float depth, const vec3 normal, const float specularFactor)
{
	const vec3 deepWaterColour = vec3(0.0, 0.0, 1.0);
	const vec3 shallowWaterColour = vec3(148.0 / 255.0, 217.0 / 255.0, 255.0 / 255.0);

	// The depth at which the colour of the water should be
	// equal to "shallowWaterColour"
	const float deepWaterColourDepth = planetRadius / 10.0;

	float colourInterpolationAmount = depth / deepWaterColourDepth;
	colourInterpolationAmount = clamp(colourInterpolationAmount, 0.0, 1.0);

	vec3 waterColour = mix(shallowWaterColour, deepWaterColour, colourInterpolationAmount);

	// vvv Specular lighting vvv

	// We are look at the scene from the camera's
	// perspective
	const vec3 toCamera = vec3(0.0, 0.0, 1.0);
	// The direction of the reflected sun ray
	const vec3 reflectedRay = reflect(TO_SUN * -1.0, normal);
	// If the reflected sun ray goes into the camera,
	// we want a high specularity
	float specular = dot(reflectedRay, toCamera);
	specular = max(specular, 0.0);
	specular = pow(specular, 10.0) * specularFactor;

	// ^^^ Specular lighting ^^^

	// vvv Diffuse lighting vvv

	// If the normal is facing the sun, we 
	// want a high brightness
	float brightness = dot(normal, TO_SUN);
	brightness = max(brightness, 0.1);

	// ^^^ Diffuse lighting ^^^


	// Apply the diffuse lighting
	waterColour.xyz *= brightness;
	// Apply the specular lighting
	waterColour.xyz += specular;

	return waterColour;
}

in vec2 uv;
out vec4 colour;
void main()
{
	// The pixel's position on the screen
	const vec3 pixelScreenPosition = vec3(uv * 2.0 - 1.0, texture(depthTexture, uv).r * 2.0 - 1.0);
	// The pixel's position in the world
	const vec3 point = ConvertPixelPositionToWorldPosition(pixelScreenPosition);

	// Things that are located along this direction are
	// seen by the player
	const vec3 sightDirection = normalize(point);
	SphereIntersections sphereIntersections = 
		GetSphereIntersections(sightDirection, planetPosition, planetRadius);

	// Sort the sphere intersections so that the first intersection
	// becomes the closest visible intersection
	sphereIntersections = SortSphereIntersections(sphereIntersections);

	// Distance to the closest visible water surface
	const float distanceToSurface = sphereIntersections.first;
	const float distanceToOtherSurface = sphereIntersections.second;
	const float distanceToPoint = length(point);

	// Only apply the water colour if the water is seen by the 
	// player ("distanceToSurface > 0.0") and if the point
	// is behind any of the water surfaces 
	// ("distanceToPosition > distanceToSurface || 
	// distanceToPosition > distanceToOtherSurface")
	if (distanceToSurface > 0.0 && 
		(distanceToPoint > distanceToSurface ||
			distanceToPoint > distanceToOtherSurface))
	{
		// The water depth that is in front
		// of the point
		float depth = 0.0;
		vec3 normal = vec3(0.0);
		float specularFactor = 0.0;

		if (distanceToPoint > distanceToSurface)
		{
			// The point is behind the surface

			const vec3 surfacePoint = sightDirection * distanceToSurface;

			const vec3 faceNormal = normalize(surfacePoint - planetPosition);
			// "GetNormalMappedNormal" needs to have the "toPointOnSphere" vector 
			// in the world space rotation. The scene is currently rotated
			// by the view rotation, so to be able to revert that, we need
			// to multiply by the inverse of the view rotation
			const vec3 toPointOnSphere = (viewRotationInverse * vec4(faceNormal, 1.0)).xyz;

			// Get the normal at two different locations on the sphere, by rotating
			// the point on the sphere by two different rotation matrices. By making
			// the rotation matrices change over time, it will look like the surfaces
			// (in this case the waves) are moving
			const vec3 waterNormal0 = GetNormalMappedNormal(normalMapRotation0 * toPointOnSphere, faceNormal);
			const vec3 waterNormal1 = GetNormalMappedNormal(normalMapRotation1 * toPointOnSphere, faceNormal);

			// Take the average of the two normals
			normal = (waterNormal0 + waterNormal1) / 2.0;
			normal = normalize(normal);
		}
		else if (distanceToOtherSurface < 0)
		{
			// The camera and the point are inside the ocean

			normal = normalize(point - planetPosition);
		}

		// We could rewrite this, to avoid conditional branching,
		// but, for the sake of clarity, we keep it in this way
		if (distanceToOtherSurface < 0)
		{
			// The camera is inside the ocean, but the point may or may not be

			depth = min(distanceToPoint, distanceToSurface);
			// Notice that we are not setting the specular factor, since
			// we do not want any specularity when the point is seen 
			// from within the ocean
		}
		else if(distanceToPoint > distanceToSurface)
		{
			// The point is behind the surface, and the camera is outside the ocean

			depth = min(distanceToPoint - distanceToSurface,
					length(distanceToSurface - distanceToOtherSurface));
			specularFactor = 0.8;
		}
		vec3 waterColour = GetWaterColour(depth, normal, specularFactor);
		vec3 sceneColour = texture(renderedSceneTexture, uv).rgb;

		// The depth at which the ocean becomes fully opaque
		const float fullyOpaqueDepth = planetRadius / 6.0;
		float opaqueness = depth / fullyOpaqueDepth;
		opaqueness = clamp(opaqueness, 0.7, 1.0);

		colour = vec4(mix(sceneColour, waterColour, opaqueness), 1.0);
	}
	else
	{
		// The point is not seen through any water so render
		// the point without any effect
		colour = vec4(texture(renderedSceneTexture, uv).rgb, 1.0);
	}
}