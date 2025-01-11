#version 430 core

Shader "Picking Shader"
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

			layout(set = 0, binding = 1) uniform UBO
			{
				vec4 id;
			} ubo;

			void main() 
			{
				outColor = ubo.id;
			}
		}
	}
}