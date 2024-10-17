#version 430 core

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform Uniform
{
	vec4 color;
} uniforms;

void main() {
    outColor = uniforms.color;
}