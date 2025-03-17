#version 430 core

Shader "Grid Shader"
{
	Property
	{
		vec4 color;
	}
	Pass
	{
		LightingPass "Forward"
		Stage Vertex
		{
			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			uniform vec4 color;

			void main() 
			{
				outColor = color;
			}
		}
	}
}