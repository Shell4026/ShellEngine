#version 430 core

Shader "Text Shader"
{
	Property
	{
		vec4 color;
		[Local] sampler2D tex;
	}
	
	Pass
	{
		LightingPass "Transparent"
		
		Cull Off;
		ZWrite Off;
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			
			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0f);
				fragUvs = UV;
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			uniform vec4 color;
			uniform sampler2D tex;

			void main() 
			{
				vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, uvs).r);
				outColor = vec4(color) * sampled;
			}
		}
	}
}