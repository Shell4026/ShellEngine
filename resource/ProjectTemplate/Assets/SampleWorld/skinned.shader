#version 430 core

Shader "Skinned Shader2"
{
	Property
	{
		float ambient;
		sampler2D tex;
	}

	Pass
	{
		LightingPass "Opaque"

		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormal;

			void main()
			{
				vec4 skinnedPos = MATRIX_SKIN * vec4(VERTEX, 1.0);
				vec3 skinnedNormal = mat3(MATRIX_SKIN) * NORMAL;

				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * skinnedPos;
				
				fragUvs = UV;
				fragPos = (MATRIX_MODEL * skinnedPos).xyz;
				fragNormal = normalize(mat3(transpose(inverse(MATRIX_MODEL))) * skinnedNormal);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormal;

			uniform float ambient;
			uniform sampler2D tex;

			void main()
			{
				float diffuse = 0.0;
				
				for (int i = 0; i < LIGHT.count; ++i)
				{
					int type = int(LIGHT.lights[i].other.w);
					if (type == 0)
					{
						vec3 toLightDir = -LIGHT.lights[i].pos.xyz;
						float intensity = LIGHT.lights[i].pos.w;
						diffuse += max(dot(fragNormal, toLightDir), 0.0) * intensity;
					}
					else if (type == 1)
					{
						vec3 toLightVec  = LIGHT.lights[i].pos.xyz - fragPos;
						vec3 toLightDir  = normalize(toLightVec);
						float lightDis   = length(toLightVec);
						float attenuation = clamp(1.0 - (lightDis / LIGHT.lights[i].pos.w), 0.0, 1.0);
						diffuse += max(dot(fragNormal, toLightDir), 0.0) * attenuation;
					}
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + ambient;
			}
		}
	}
}
