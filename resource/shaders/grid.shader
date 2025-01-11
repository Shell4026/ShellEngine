#version 430 core

Shader "Grid Shader"
{
	Pass
	{
		Stage Vertex
		{
			layout(location = 0) in vec3 verts;

			layout(set = 0, binding = 0) uniform MVP
			{
				mat4 model;
				mat4 view;
				mat4 proj;
			} mvp;

			void main()
			{
				gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(set = 1, binding = 0) uniform Uniform
			{
				vec4 color;
			} uniforms;

			void main() 
			{
				outColor = uniforms.color;
			}
		}
	}
}