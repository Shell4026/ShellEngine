#version 430 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uvs;

layout(binding = 2) uniform sampler2D tex;

void main() {
    outColor = texture(tex, uvs);
}