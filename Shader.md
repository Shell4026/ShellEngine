# 개요
코드를 통해 렌더링 파이프라인을 설정 할 필요 없이, 셰이더를 통해 처리 할 수 있습니다.

Unity엔진의 ShaderLab + GLSL의 문법을 합친 것이 특징 입니다.

# 상세 원리
1. ShaderLexer클래스로 코드를 토큰 단위로 나눕니다
2. ShaderParser클래스로 토큰을 하나하나 읽어 문법을 검사하고 파싱 트리 구조를 완성해나갑니다.
3. ShaderParse내 Generate함수로 파싱 트리를 기반으로 GLSL문법으로 변환한 후 셰이더를 컴파일 합니다.

## 예시 코드

```glsl
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
		LightingPass "Forward"
		
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
```
### 스텐실을 이용한 아웃라인 셰이더 예시
```glsl
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
```
### 로컬 키워드를 사용해 오브젝트별 텍스쳐 설정 가능한 반투명 셰이더 예시
로컬 키워드로 된 유니폼 변수는 MeshRenderer MaterialPropertyBlock을 지정해준 후 값을 넣어주면 됩니다.

```glsl
#version 430 core
Shader "Unlit Transparent Shader2"
{
	Property
	{
		vec3 color;
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
			uniform vec3 color;
			uniform sampler2D tex;

			void main() 
			{
				outColor = texture(tex, uvs);
				outColor.xyz *= color.xyz;
			}
		}
	}
}
```
