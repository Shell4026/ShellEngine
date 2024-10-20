#version 430 core

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform UBO
{
	vec4 id;
} ubo;

void main() {
    outColor = ubo.id;
}