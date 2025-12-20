#version 430 core

Shader "Triangle Shader"
{
	Property
	{
		[Local] vec2 offset;
		vec3 color;
	}
	
	Pass
	{
		LightingPass "Opaque"
		
		Stage Vertex
		{
			uniform vec2 offset;
			
			void main()
			{
				vec4 vert = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
				gl_Position = vert;
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			uniform vec3 color;
			
			void main() 
			{
				outColor = vec4(color, 1.0);
			}
		}
	}
}