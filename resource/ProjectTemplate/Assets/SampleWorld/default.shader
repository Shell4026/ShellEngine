#version 430 core

Shader "Default Shader"
{
	Property
	{
		float ambient;
		sampler2D tex;
		sampler2D normalTex;
	}
	Pass
    {
		LightingPass "ShadowMapPass"
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
            void main()
            {
            }
        }
    }
	Pass
	{
		LightingPass "Opaque"
		
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out mat3 TBN;
			void main()
			{
				fragUvs = UV;
				fragPos = (MATRIX_MODEL * vec4(VERTEX, 1.0f)).xyz;
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * vec4(fragPos, 1.0f);
				
				mat3 normalMatrix = transpose(inverse(mat3(MATRIX_MODEL)));
				
				vec3 t = normalize(normalMatrix * TANGENT);
				vec3 n = normalize(normalMatrix * NORMAL);
				vec3 b = cross(n, t);

				TBN = mat3(t, b, n);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in mat3 TBN;
			uniform float ambient;
			uniform sampler2D tex;
			uniform sampler2D normalTex;

			float ShadowSample(vec3 worldPos, float nDotL, vec4 shadowRect, mat4 lightMatrix)
			{
				const vec4 lightClip = lightMatrix * vec4(worldPos, 1.0);
				const vec3 proj = lightClip.xyz / lightClip.w;
				if (proj.z > 1.0 || proj.z < 0.0) 
					return 1.0;
				
				vec2 tileUV;
				tileUV.x = proj.x * 0.5 + 0.5;
				tileUV.y = 1.0 - (proj.y * 0.5 + 0.5);
				if (tileUV.x < 0.0 || tileUV.x > 1.0 || tileUV.y < 0.0 || tileUV.y > 1.0)
					return 1.0;
				const vec2 uv = tileUV * shadowRect.zw + shadowRect.xy;
				
				const float currentDepth = proj.z;
				const vec2 atlasSize = textureSize(TEXTURE_SHADOW, 0);
				const vec2 texelSize = 1.0 / (atlasSize);
				
				const float bias = max(0.005 * (1.0 - nDotL), 0.001);
				float result = 0.0;
				for (int x = -1; x <= 1; ++x)
				{
					for (int y = -1; y <= 1; ++y)
					{
						float pcfDepth = texture(TEXTURE_SHADOW, uv + vec2(x, y) * texelSize).r;
						result += (currentDepth - bias) > pcfDepth ? 0.0 : 1.0;
					}
				}
				result /= 9.0;
				return result;
			}
	
			void main() 
			{
				vec3 normal = texture(normalTex, uvs).xyz;
				normal = normal * 2.0 - 1.0;
				normal = normalize(TBN * normal);
			
				float diffuse = 0.0;
				for (int i = 0; i < LIGHT.count; ++i)
				{
					int type = int(LIGHT.lights[i].other.w);
					if (type == 0)
					{
						vec3 toLightDir = normalize(-LIGHT.lights[i].pos.xyz);
						const float intensity = LIGHT.lights[i].pos.w;
						const float nDotL = max(dot(normal, toLightDir), 0.0);
						if (nDotL > 0.0) 
						{
							const float shadow = ShadowSample(fragPos, nDotL, LIGHT.lights[i].shadowRect, LIGHT.lights[i].lightSpaceMatrix);
							diffuse += nDotL * intensity * shadow;
						}
					}
					else if (type == 1)
					{
						vec3 toLightVec = LIGHT.lights[i].pos.xyz - fragPos;
						vec3 toLightDir = normalize(toLightVec);
						float lightDis = length(toLightVec);
						float attenuation = clamp(1.0 - (lightDis / LIGHT.lights[i].pos.w), 0.0, 1.0);
						const float nDotL = max(dot(normal, toLightDir), 0.0);
						if (nDotL > 0.0) 
						{
							const float shadow = ShadowSample(fragPos, nDotL, LIGHT.lights[i].shadowRect, LIGHT.lights[i].lightSpaceMatrix);
							diffuse += nDotL * attenuation * shadow;
						}
					}
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + ambient;
			}
		}
	}
}