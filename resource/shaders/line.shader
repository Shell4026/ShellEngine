#version 430 core

Shader "Line Shader"
{
	Pass
	{
		Stage Vertex
		{
			layout(set = 0, binding = 0) uniform MVP
			{
				mat4 model;
				mat4 view;
				mat4 proj;
			} mvp;
			layout(set = 0, binding = 1) uniform Points
			{
				vec3 start;
				vec3 end;
			} points;

			void main()
			{
				vec3 point = points.start;
				if(gl_VertexIndex == 1)
					point = points.end;
				gl_Position = mvp.proj * mvp.view * mvp.model * vec4(point, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(set = 0, binding = 2) uniform Color
			{
				vec4 color;
			} color;

			void main() {
				outColor = color.color;
			}
		}
	}
}