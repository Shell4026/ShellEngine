#version 430 core

Shader "SSAO Shader"
{
	Property
	{
		vec3 kernel[64];
		sampler2D normalTex;
		sampler2D depthTex;
		sampler2D noiseTex;
		sampler2D aoTex;
	}
	
	Pass
	{
		LightingPass "SSAOPass"
		Cull Back;
		
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUV;
			void main()
			{
				fragUV = UV;
				gl_Position = vec4(VERTEX, 1.0f);
			}
		}
		Stage Fragment
		{
			layout(location = 0) in vec2 uvs;
			
			layout(location = 0) out vec4 outColor;
			
			uniform vec3 kernel[64];
			uniform sampler2D depthTex;
			uniform sampler2D normalTex;
			uniform sampler2D noiseTex;

			vec3 ReconstructViewPos(vec2 uv, float depth)
			{
				vec4 clipPos = vec4(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1.0, depth, 1.0);
				vec4 viewPos = inverse(MATRIX_PROJ) * clipPos;
				viewPos /= viewPos.w;
				return viewPos.xyz;
			}

			void main() 
			{
				float depth = texture(depthTex, uvs).r;

				if (depth >= 1.0)
				{
					outColor = vec4(1.0);
					return;
				}
				vec3 fragPos = ReconstructViewPos(uvs, depth);
				vec3 normal = texture(normalTex, uvs).xyz;
				normal = normalize(normal * 2.0 - 1.0);
				
				const vec2 noiseScale = vec2(1024.0/4.0, 768.0/4.0);
				vec3 randomVec = normalize(vec3(texture(noiseTex, uvs * noiseScale).rg, 0.0));
				
				vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
				vec3 bitangent = cross(normal, tangent);
				mat3 TBN = mat3(tangent, bitangent, normal);
				
				float occlusion = 0.0;
				
				for (int i = 0; i < 64; ++i)
				{
					const float radius = 0.5f;
					vec3 samplePos = fragPos + TBN * kernel[i] * radius;

					vec4 offset = MATRIX_PROJ * vec4(samplePos, 1.0);
					offset.xyz /= offset.w;
					offset.xy = vec2(offset.x * 0.5 + 0.5, 0.5 - offset.y * 0.5);

					if (offset.x < 0.0 || offset.x > 1.0 ||
						offset.y < 0.0 || offset.y > 1.0)
					{
						continue;
					}

					float sampleDepth = texture(depthTex, offset.xy).r;
					vec3 sampleViewPos = ReconstructViewPos(offset.xy, sampleDepth);

					float rangeCheck = smoothstep(
						0.0,
						1.0,
						radius / abs(fragPos.z - sampleViewPos.z)
					);
					
					const float bias = 0.025f;
					occlusion += (sampleViewPos.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
				}

				const float strength = 1.5f;
				occlusion = 1.0 - (occlusion / 64.0);
				occlusion = pow(occlusion, strength);

				outColor = vec4(vec3(occlusion), 1.0);
			}
		}
	}
	Pass
	{
		LightingPass "CombinePass"
		Cull Off;
		ZWrite Off;
		ZTest Off;

		Stage Vertex
		{
			layout(location = 0) out vec2 fragUV;
			void main()
			{
				fragUV = UV;
				gl_Position = vec4(VERTEX, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) in vec2 uvs;

			layout(location = 0) out vec4 outColor;

			uniform sampler2D aoTex;

			void main()
			{
				float ao = texture(aoTex, uvs).r;
				float shadow = 1.0 - ao;
				if (shadow <= 0.001)
					discard;
				outColor = vec4(0.0, 0.0, 0.0, shadow);
			}
		}
	}
}
