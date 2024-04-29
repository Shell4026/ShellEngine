#version 430 core

layout(location = 0) out vec3 fragColor;

vec3 pos[3] = vec3[](
	vec3(0.0, -0.5, 0.0),
	vec3(0.5, 0.5, 0.0),
	vec3(-0.5, 0.5, 0.0)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main()
{
	gl_Position = vec4(pos[gl_VertexIndex], 1.0);
	fragColor = colors[gl_VertexIndex];
}