#version 430 core

Shader "Unlit Shader"
{
	Property
	{
		sampler2D tex;
	}
	
	Pass
	{
		LightingPass "Opaque"
		
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
			uniform sampler2D tex;

			void main() 
			{
				outColor = texture(tex, uvs);
			}
		}
	}
}