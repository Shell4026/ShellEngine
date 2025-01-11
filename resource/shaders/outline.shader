#version 430 core

Shader "Outline Shader"
{
	Pass
	{
		Stage Vertex
		{
			layout(location = 0) in vec3 verts;
			layout(location = 1) in vec3 normals;

			layout(set = 0, binding = 0) uniform MVP
			{
				mat4 model;
				mat4 view;
				mat4 proj;
			} mvp;

			layout(set = 1, binding = 0) uniform UBO
			{
				float outlineWidth;
			} ubo;

			void main()
			{
				vec4 vert = vec4(verts + normals * ubo.outlineWidth, 1.0f);
				gl_Position = mvp.proj * mvp.view * mvp.model * vert;
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(set = 1, binding = 1) uniform Material
			{
				vec4 color;
			} material;

			void main() 
			{
				outColor = material.color;
			}
		}
	}
}