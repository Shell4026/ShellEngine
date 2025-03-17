#version 430 core

Shader "Outline Shader"
{
	Property
	{
		float outlineWidth;
		vec4 color;
	}
	Pass
	{
		LightingPass "Forward"
		ColorMask 0;
		
		Stencil
		{
			Ref 1;
			ReadMask 255;
			WriteMask 255;
			Comp Always;
			Pass Replace;
			Fail Replace;
			ZFail Replace;
		}
		
		Stage Vertex
		{
			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0f);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;
			void main()
			{
				outColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}
	Pass
	{
		LightingPass "Forward"
		Stencil
		{
			Ref 1;
			ReadMask 255;
			WriteMask 255;
			Comp NotEqual;
			Pass Replace;
			Fail Keep;
			ZFail Keep;
		}
		Cull Back;
		
		Stage Vertex
		{
			uniform float outlineWidth;

			void main()
			{
				vec4 vert = vec4(VERTEX + NORMAL * outlineWidth, 1.0f);
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vert;
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