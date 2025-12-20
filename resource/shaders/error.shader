#version 430 core

Shader "Error Shader"
{
	Pass
	{
		LightingPass "Opaque"
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

			void main() 
			{
				outColor = vec4(0.98, 0.55, 0.89, 1.0);
			}
		}
	}
}