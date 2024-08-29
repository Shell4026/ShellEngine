#version 430 core
layout(location = 0) in vec3 verts;

layout(binding = 0) uniform MVP
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

void main()
{
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0);
}