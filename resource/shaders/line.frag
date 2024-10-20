#version 430 core

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform Color
{
	vec4 color;
} color;

void main() {
    outColor = color.color;
}