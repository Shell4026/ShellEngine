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
		LightingPass "Opaque"
		
		Stage Vertex
		{
			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out mat3 TBN;
			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0f);
				fragUvs = UV;
				fragPos = (MATRIX_MODEL * vec4(VERTEX, 1.0f)).xyz;
				
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

			void main() 
			{
				vec3 normal = texture(normalTex, uvs).xyz;
				normal = normal * 2.0 - 1.0;
				normal = normalize(TBN * normal);
			
				float diffuse = 0.0;
				for (int i = 0; i < LIGHT.count; ++i)
				{
					int type = int(LIGHT.other[i].w);
					if (type == 0)
					{
						vec3 toLightDir = -LIGHT.pos[i].xyz;
						float intensity = LIGHT.pos[i].w;
						diffuse += max(dot(normal, toLightDir), 0.0) * intensity;
					}
					else if (type == 1)
					{
						vec3 toLightVec = LIGHT.pos[i].xyz - fragPos;
						vec3 toLightDir = normalize(toLightVec);
						float lightDis = length(toLightVec);
						float attenuation = clamp(1.0 - (lightDis / LIGHT.pos[i].w), 0.0, 1.0);
						diffuse += max(dot(normal, toLightDir), 0.0) * attenuation;
					}
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + ambient;
			}
		}
	}
}