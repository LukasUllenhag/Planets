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

layout(binding = 0) uniform sampler2D sampler;

in vec2 uv;
out vec4 colour;
void main()
{
	colour = texture(sampler, uv);
}
