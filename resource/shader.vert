#version 430 core
layout(location = 0) in vec3 verts;
layout(location = 1) in vec4 colors;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform MVP
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

void main()
{
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0);
	fragColor = colors;
}