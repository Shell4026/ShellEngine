#version 430 core

Shader "Picking Shader"
{
	Property
	{
		[Local] vec4 id;
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

			uniform vec4 id;

			void main() 
			{
				outColor = id;
			}
		}
	}
}