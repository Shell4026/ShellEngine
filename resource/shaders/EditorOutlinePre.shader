#version 430 core

Shader "Editor Outline PreProcess Shader"
{
	Pass
	{
		LightingPass "EditorOutline"
		Cull Off;
		
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
				outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
}