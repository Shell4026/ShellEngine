#version 430 core
layout(location = 0) in vec3 verts;
//layout(location = 1) in vec3 colors;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = vec4(verts, 1.0);
	fragColor = vec3(1.0, 0.0, 0.0);
}