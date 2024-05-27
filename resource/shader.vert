#version 430 core
layout(location = 0) in vec3 verts;
layout(location = 1) in vec4 colors;
layout(location = 2) in vec2 uvs;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUvs;

layout(binding = 0) uniform MVP
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

layout(binding = 1) uniform Offset
{
	vec3 offset1;
	float offset2;
} offset;

void main()
{
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts + offset.offset1, 1.0);
	fragColor = colors + offset.offset2;
	fragUvs = uvs;
}