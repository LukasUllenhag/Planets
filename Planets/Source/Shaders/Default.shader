#Shader Vertex
#version 450 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) uniform vec3 cameraPosition;
layout(location = 1) uniform mat4 rotationMatrix;
layout(location = 2) uniform mat4 projectionMatrix;
layout(location = 4) uniform vec3 worldPosition;
layout(location = 5) uniform float scale;

out VS_OUT
{
	vec2 uv;
	float brightness;
	vec3 normal;
	vec3 toCamera;
}vsOut;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

void main()
{
	vec3 position = worldPosition + vertexPosition * scale;
	vsOut.uv = uv;
	vsOut.normal = normal;
	vsOut.toCamera = normalize(cameraPosition - position);

	// Diffuse lighting
	float brightness = dot(normal, TO_SUN);
	vsOut.brightness = max(brightness, 0.3);

	gl_Position = projectionMatrix * rotationMatrix * vec4(position - cameraPosition, 1.0);
}

#Shader Fragment
#version 450 core

layout(binding = 1) uniform sampler2D sampler;

in VS_OUT
{
	vec2 uv;
	float brightness;
	vec3 normal;
	vec3 toCamera;
} fsIn;

const vec3 TO_SUN = normalize(vec3(1.0, 5.0, 0.0));

out vec4 colour;

void main()
{
	// Specular lighting
	vec3 toCamera = normalize(fsIn.toCamera);
	vec3 normal = normalize(fsIn.normal);
	vec3 reflectedRay = reflect(TO_SUN * -1.0, normal);
	float specular = dot(reflectedRay, toCamera);
	specular = max(specular, 0.0);
	specular = pow(specular, 10.0);


	colour = texture(sampler, fsIn.uv);
	colour.xyz *= fsIn.brightness;
	colour.xyz += specular;
}
