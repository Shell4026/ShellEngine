#version 430 core
layout(location = 0) in vec3 verts;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec2 fragUvs;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormals;

layout(set = 0, binding = 0) uniform MVP
{
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

void main()
{
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0f);
	fragUvs = uvs;
	fragPos = (mvp.model * vec4(verts, 1.0f)).xyz;
	fragNormals = normalize((mvp.model * vec4(normals, 0.0f)).xyz);
}