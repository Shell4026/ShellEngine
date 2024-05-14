#version 430 core

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 fragColor;

void main() {
    outColor = fragColor;
}