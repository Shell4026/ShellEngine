#version 430 core

Shader "Editor Outline PostProcess Shader"
{
	Property
	{
		float outlineWidth;
		vec4 outlineColor;
		sampler2D tex;
	}
	Pass
	{
		LightingPass "EditorPostOutline"
		Cull Off;
		
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			void main()
			{
				fragUvs = UV;
				gl_Position = vec4(VERTEX, 1.0f);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;
			layout(location = 0) in vec2 uvs;
			uniform sampler2D tex;
			uniform vec4 outlineColor;
			uniform float outlineWidth;

			float SobelFilter(float stepx, float stepy, vec2 center)
			{
				float tleft = texture(tex, center + vec2(-stepx,stepy)).r;
				float left = texture(tex, center + vec2(-stepx,0)).r;
				float bleft = texture(tex, center + vec2(-stepx,-stepy)).r;
				float top = texture(tex, center + vec2(0,stepy)).r;
				float bottom = texture(tex, center + vec2(0,-stepy)).r;
				float tright = texture(tex, center + vec2(stepx,stepy)).r;
				float right = texture(tex, center + vec2(stepx,0)).r;
				float bright = texture(tex, center + vec2(stepx,-stepy)).r;
				
				float x = tleft + 2.0 * left + bleft - tright - 2.0 * right - bright;
				float y = -tleft - 2.0 * top - tright + bleft + 2.0 * bottom + bright;
				float str = sqrt((x*x) + (y*y));
				return str;
			}
			
			void main() 
			{
				float intensity = SobelFilter(outlineWidth / 512, outlineWidth / 512, uvs);
				outColor = outlineColor;
				if (intensity < 0.1)
					discard;
				outColor.a = intensity;
			}
		}
	}
}