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
	vec3 normal;
	vec3 toCamera;
}vsOut;

void main()
{
	const vec3 position = worldPosition + vertexPosition * scale;
	vsOut.normal = normal;
	vsOut.toCamera = normalize(cameraPosition - position);

	gl_Position = projectionMatrix * viewRotation * vec4(position - cameraPosition, 1.0);
}

#Shader Fragment
#version 450 core

in VS_OUT
{
	vec3 normal;
	vec3 toCamera;
} fsIn;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

out vec4 colour;

void main()
{
	const vec3 normal = normalize(fsIn.normal);
	
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

	colour = vec4(153.0 / 255.0, 133.0 / 255.0, 122.0 / 255.0, 1.0);

	// Apply the diffuse lighting
	colour.xyz *= brightness;
	// Apply the specular lighting
	colour.xyz += specular;
}
