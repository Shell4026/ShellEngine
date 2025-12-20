#version 430 core

Shader "Line Shader"
{
	Property
	{
		[Local] vec3 start;
		[Local] vec3 end;
		[Local] vec4 color;
	}
	Pass
	{
		LightingPass "Opaque"
		
		Stage Vertex
		{
			uniform vec3 start;
			uniform vec3 end;
			
			void main()
			{
				vec3 point = start;
				if(gl_VertexIndex == 1)
					point = end;
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(point, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			uniform vec4 color;

			void main() {
				outColor = color;
			}
		}
	}
}