#version 430 core

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform Uniforms
{
	vec4 color;
} uniforms;

void main() {
    outColor = uniforms.color;
}