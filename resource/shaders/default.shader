#version 430 core

Shader "Default Shader"
{
	Property
	{
		float ambient;
		sampler2D tex;
	}
	
	Pass
	{
		LightingPass "Forward"
		
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormals;

			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0f);
				fragUvs = UV;
				fragPos = (MATRIX_MODEL * vec4(VERTEX, 1.0f)).xyz;
				fragNormals = normalize((MATRIX_MODEL * vec4(NORMAL, 0.0f)).xyz);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormals;

			uniform float ambient;
			uniform sampler2D tex;

			void main() 
			{
				float diffuse = 0.f;
				for (int i = 0; i < LIGHT.count; ++i)
				{
					vec3 toLightVec = LIGHT.pos[i].xyz - fragPos;
					vec3 toLightDir = normalize(toLightVec);
					float lightDis = length(toLightVec);
					float attenuation = clamp(1.0 - (lightDis / LIGHT.range[i]), 0.0, 1.0);
					
					diffuse += max(dot(fragNormals, toLightDir), 0.0) * attenuation;
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + ambient;
			}
		}
	}
}