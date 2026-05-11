# Shader 문서

ShellEngine의 `.shader` 파일은 Unity ShaderLab처럼 렌더 상태와 패스를 선언하고, 각 `Stage` 안에는 GLSL 코드를 직접 작성하는 형식입니다. 엔진은 이 파일을 파싱해 Vulkan용 GLSL을 생성하고, `glslc`로 SPIR-V를 컴파일한 뒤 `Shader`, `ShaderPass`, 머티리얼 유니폼 레이아웃으로 연결합니다.

관련 구현은 주로 다음 파일에 있습니다.

- `src/Render/ShaderLexer.cpp`: 소스 문자열을 토큰으로 분해
- `src/Render/ShaderParser.cpp`: 토큰을 AST로 파싱하고 GLSL 코드 생성
- `src/Render/ShaderGenerator.cpp`: 패스별 `.vert`, `.frag` 임시 파일 생성
- `src/Game/Asset/ShaderLoader.cpp`: `.shader` 로드, `glslc` 실행, SPIR-V 로드
- `src/Render/ShaderPass.cpp`: 파싱 결과에서 어트리뷰트/유니폼 레이아웃 구성
- `src/Render/VulkanImpl/VulkanShaderPass.cpp`: Vulkan 디스크립터 세트와 파이프라인 레이아웃 생성

## 처리 흐름

1. `ShaderLoader::Load(path)`가 `.shader` 파일을 텍스트로 읽습니다.
2. `ShaderLexer::Lex()`가 키워드, 식별자, 숫자, 문자열, 연산자, 중괄호 등을 토큰으로 나눕니다.
3. `ShaderParser::Parse()`가 토큰을 `ShaderAST::ShaderNode`로 변환합니다.
4. 파서는 `Stage` 본문을 분석하며 `VERTEX`, `MATRIX_PROJ`, `LIGHT` 같은 엔진 예약 심볼을 발견하면 필요한 입력 어트리뷰트와 버퍼를 자동으로 추가합니다.
5. `GenerateStageCode()`가 각 `Stage`의 최종 GLSL 코드를 만듭니다.
6. `ShaderGenerator::GenerateShaderFile()`이 패스별 `.vert`, `.frag` 파일을 캐시 경로에 저장합니다.
7. `ShaderLoader`가 `glslc`로 각 스테이지를 `.spv`로 컴파일합니다.
8. `ShaderPassBuilder`가 SPIR-V를 `ShaderPass`로 만들고, Vulkan 구현에서는 `VkShaderModule`, descriptor set layout, pipeline layout을 구성합니다.
9. 최종 `Shader` 객체는 프로퍼티 목록과 패스 목록을 보관하고, `Material`은 이를 기준으로 유니폼/샘플러 데이터를 업로드합니다.

## 기본 구조

```glsl
#version 430 core

Shader "Shader Name"
{
	Property
	{
		float ambient;
		vec4 color;
		sampler2D albedo;
	}

	Pass
	{
		LightingPass "Opaque"
		Cull Back;
		ZWrite On;
		ZTest On;

		Stage Vertex
		{
			layout(location = 0) out vec2 fragUV;

			void main()
			{
				fragUV = UV;
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
			}
		}

		Stage Fragment
		{
			layout(location = 0) in vec2 fragUV;
			layout(location = 0) out vec4 outColor;

			uniform float ambient;
			uniform vec4 color;
			uniform sampler2D albedo;

			void main()
			{
				outColor = texture(albedo, fragUV) * color;
				outColor.rgb += ambient;
			}
		}
	}
}
```

최상단 `#version`은 현재 파서에서 `#version 430 core` 형태를 기대합니다. `Shader "..."`는 생성될 `Shader` 객체의 이름입니다. `Property`는 머티리얼에서 조작할 수 있는 값 목록이며 생략할 수 있습니다. `Pass`는 하나 이상 선언할 수 있고, 각 패스는 렌더 패스 이름과 렌더 상태, `Vertex`/`Fragment` 스테이지를 가집니다.

## Property

`Property`에 선언한 변수는 `Material::SetProperty()`로 값을 넣을 수 있는 머티리얼 프로퍼티가 됩니다. 스테이지 안에서 `uniform`으로 사용하려면 같은 타입과 이름이 `Property`에 먼저 선언되어 있어야 합니다.

지원되는 주요 타입은 다음과 같습니다.

| Shader 타입 | C++ 저장 타입 |
| --- | --- |
| `int` | `int` |
| `float` | `float` |
| `vec2` | `glm::vec2` |
| `vec3` | `glm::vec3` |
| `vec4` | `glm::vec4` |
| `mat3` | `glm::mat3` |
| `mat4` | `glm::mat4` |
| `sampler2D` | `Texture` |

배열도 선언할 수 있습니다.

```glsl
Property
{
	vec3 kernel[64];
	sampler2D normalTex;
}
```

### Local 프로퍼티

`[Local]`을 붙인 프로퍼티는 머티리얼 공통 값이 아니라 오브젝트별 값으로 취급됩니다. 예를 들어 같은 머티리얼을 공유하되 오브젝트마다 텍스처를 다르게 넣고 싶을 때 사용합니다.

```glsl
Property
{
	vec4 color;
	[Local] sampler2D tex;
}
```

일반 프로퍼티는 `UniformStructLayout::Usage::Material` 세트로, `[Local]` 프로퍼티는 `UniformStructLayout::Usage::Object` 세트로 배치됩니다. 런타임에서는 `MaterialData`와 `MaterialPropertyBlock`이 이 구분을 기준으로 버퍼와 샘플러를 갱신합니다.

## Pass

`Pass`는 실제 draw에 사용되는 렌더링 단위입니다. 같은 `LightingPass` 이름을 가진 패스들은 `Shader::GetShaderPasses(name)`로 조회되어 해당 렌더 단계에서 순서대로 실행됩니다.

```glsl
Pass "Optional Pass Name"
{
	LightingPass "Opaque"
	Cull Back;
	ZWrite On;
	ZTest On;
	ColorMask RGBA;

	Stage Vertex { ... }
	Stage Fragment { ... }
}
```

패스 이름은 생략할 수 있습니다. 생략하면 파서가 `Pass1`, `Pass2`처럼 자동 이름을 붙입니다.

### 렌더 상태

| 구문 | 값 | 기본값 | 설명 |
| --- | --- | --- | --- |
| `LightingPass "name"` | 문자열 | 빈 이름 | 렌더러가 패스를 찾을 때 쓰는 분류 이름 |
| `Cull` | `Off`, `Front`, `Back` | `Back` | 컬링 모드 |
| `ZWrite` | `On`, `Off` | `On` | 깊이 버퍼 쓰기 여부 |
| `ZTest` | `On`, `Off` | `On` | 깊이 테스트 여부 |
| `ColorMask` | `RGBA` 조합 또는 숫자 | `15` | 컬러 채널 쓰기 마스크 |

`ColorMask`는 비트마스크로 저장됩니다. `R=1`, `G=2`, `B=4`, `A=8`이며 `ColorMask 0;`은 컬러 출력을 막을 때 사용합니다.

### Stencil

```glsl
Stencil
{
	Ref 1;
	ReadMask 255;
	WriteMask 255;
	Comp Always;
	Pass Replace;
	Fail Keep;
	ZFail Keep;
}
```

`Comp` 값은 `Never`, `Less`, `Equal`, `LessEqual`, `Greater`, `NotEqual`, `GreaterEqual`, `Always`를 사용할 수 있습니다. `Pass`, `Fail`, `ZFail` 값은 `Keep`, `Zero`, `Replace`, `IncrementClamp`, `DecrementClamp`, `Invert`, `IncrementWrap`, `DecrementWrap`를 사용할 수 있습니다.

## Stage

현재 그래픽 셰이더는 `Stage Vertex`와 `Stage Fragment`를 사용합니다. 각 `Stage` 내부는 GLSL과 거의 같은 문법을 쓰지만, 파서가 일부 선언과 예약 심볼을 인식해 최종 GLSL 앞부분을 자동 생성합니다.

```glsl
Stage Vertex
{
	layout(location = 0) out vec2 fragUV;

	const float scale = 1.0;

	void main()
	{
		fragUV = UV;
		gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX * scale, 1.0);
	}
}
```

### layout 입출력

`layout(location = N) in/out ...;` 구문은 스테이지 입출력으로 파싱됩니다.

```glsl
layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;
```

버텍스 스테이지에서 엔진 예약 입력(`VERTEX`, `UV`, `NORMAL` 등)을 사용하면 직접 `in`을 선언하지 않아도 파서가 자동으로 입력 어트리뷰트를 추가합니다.

## 엔진 예약 심볼

예약 심볼은 셰이더 코드 안에서 일반 GLSL 변수처럼 사용할 수 있습니다. 파서는 사용 여부를 감지해 필요한 layout, uniform buffer, storage buffer, push constant를 자동으로 생성합니다.

| 심볼 | 타입/의미 | 자동 생성 |
| --- | --- | --- |
| `VERTEX` | `vec3`, 정점 위치 | vertex input location 0 |
| `UV` | `vec2`, UV | vertex input location 1 |
| `NORMAL` | `vec3`, 노멀 | vertex input location 2 |
| `TANGENT` | `vec3`, 탄젠트 | vertex input location 3 |
| `BONE_INDICES` | `ivec4`, 본 인덱스 | skinned input location 4 |
| `BONE_WEIGHTS` | `vec4`, 본 가중치 | skinned input location 5 |
| `MATRIX_MODEL` | 모델 행렬 | push constant `CONSTANTS.model` |
| `MATRIX_VIEW` | 카메라 view 행렬 | camera UBO `CAMERA.view` |
| `MATRIX_PROJ` | 카메라 projection 행렬 | camera UBO `CAMERA.proj` |
| `CAMERA` | 카메라 UBO 인스턴스 | `view`, `proj`, `pos` 멤버 |
| `LIGHT` | 라이트 SSBO | object set의 storage buffer |
| `SKIN` | 스키닝 SSBO | object set의 storage buffer |
| `MATRIX_SKIN` | 스키닝 행렬 | `BONE_*`와 `SKIN.ibm[]` 기반 계산식 삽입 |
| `TEXTURE_SHADOW` | 그림자 텍스처 | object set의 local sampler |

`MATRIX_MODEL`, `MATRIX_VIEW`, `MATRIX_PROJ`는 최종 GLSL에서 각각 `CONSTANTS.model`, `CAMERA.view`, `CAMERA.proj`로 치환됩니다.

### LIGHT 버퍼 형태

현재 파서는 `LIGHT` 사용 시 다음과 같은 구조를 생성합니다.

```glsl
struct Light
{
	vec4 pos;
	vec4 other;
	vec4 shadowRect;
	mat4 lightSpaceMatrix;
};

layout(std430, set = 1, binding = N) readonly buffer UNIFORM_LIGHT
{
	int count;
	Light lights[];
} LIGHT;
```

따라서 코드에서는 `LIGHT.count`, `LIGHT.lights[i].pos`, `LIGHT.lights[i].other`처럼 접근합니다.

```glsl
for (int i = 0; i < LIGHT.count; ++i)
{
	int type = int(LIGHT.lights[i].other.w);
	vec3 lightPos = LIGHT.lights[i].pos.xyz;
}
```

### MATRIX_SKIN

`MATRIX_SKIN`을 사용하면 파서가 `BONE_WEIGHTS`, `BONE_INDICES`, `SKIN` 버퍼를 자동 등록하고 함수 본문 앞에 다음 계산식을 삽입합니다.

```glsl
mat4 MATRIX_SKIN =
	BONE_WEIGHTS.x * SKIN.ibm[BONE_INDICES.x] +
	BONE_WEIGHTS.y * SKIN.ibm[BONE_INDICES.y] +
	BONE_WEIGHTS.z * SKIN.ibm[BONE_INDICES.z] +
	BONE_WEIGHTS.w * SKIN.ibm[BONE_INDICES.w];
```

스키닝 셰이더에서는 보통 다음처럼 사용합니다.

```glsl
vec4 skinnedPos = MATRIX_SKIN * vec4(VERTEX, 1.0);
vec3 skinnedNormal = mat3(MATRIX_SKIN) * NORMAL;
```

## Uniform 바인딩 규칙

스테이지 내부의 단일 `uniform` 선언은 반드시 `Property`에 같은 타입과 이름으로 선언되어 있어야 합니다.

```glsl
Property
{
	float roughness;
	sampler2D albedo;
}

Stage Fragment
{
	uniform float roughness;
	uniform sampler2D albedo;
}
```

파서는 스칼라/벡터/행렬 프로퍼티를 `UBO`라는 uniform block으로 묶습니다. `sampler2D`는 별도의 combined image sampler binding이 됩니다.

일반 프로퍼티는 material set에 배치되고, `[Local]` 프로퍼티는 object set에 배치됩니다. 내부 enum 기준은 다음과 같습니다.

| Usage | set | 용도 |
| --- | --- | --- |
| `Camera` | `0` | 카메라 공통 버퍼 |
| `Object` | `1` | 오브젝트별 값, push/local/lighting/skinning |
| `Material` | `2` | 머티리얼 공유 값 |

명시적 descriptor layout도 선언할 수 있습니다. 이 경우 `Property` 검사를 거치지 않고 직접 버퍼 또는 샘플러 layout으로 추가됩니다.

```glsl
layout(set = 2, binding = 0) uniform MyBlock
{
	vec4 color;
	float strength;
} customData;

layout(set = 2, binding = 1) uniform sampler2D customTex;
```

UBO는 `std140`, SSBO는 `std430` 기준으로 `UniformStructLayout`이 CPU 측 offset과 size를 계산합니다. `ShaderParser::Optimize()`는 자동 생성된 `UBO` 멤버를 스칼라, `vec2`, 그 외 큰 타입 순서로 정렬해 패딩을 줄입니다.

## constexpr와 specialization constant

스테이지 안에서 `constexpr`를 선언하면 Vulkan specialization constant로 변환됩니다.

```glsl
Stage Fragment
{
	constexpr int SAMPLE_COUNT = 16;

	void main()
	{
		for (int i = 0; i < SAMPLE_COUNT; ++i)
		{
			...
		}
	}
}
```

생성되는 GLSL은 `layout (constant_id = N) const ...` 형태입니다. 런타임에서는 `Material::SetConstant(name, value)`가 패스별 constant 데이터를 갱신하고, `VulkanPipelineManager`는 constant 데이터 해시를 파이프라인 캐시 키에 포함합니다.

현재 `ShaderPass`가 크기를 계산하는 specialization constant 타입은 `bool`, `int`, `float` 계열입니다.

## 생성되는 GLSL 예시

작성한 코드:

```glsl
Stage Vertex
{
	layout(location = 0) out vec2 fragUV;

	void main()
	{
		fragUV = UV;
		gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
	}
}
```

대략 다음과 같은 GLSL로 확장됩니다.

```glsl
#version 430 core
layout(location = 0) in vec3 VERTEX;
layout(location = 1) in vec2 UV;
layout(location = 0) out vec2 fragUV;

layout(set = 0, binding = 0) uniform UNIFORM_CAMERA
{
	mat4 view;
	mat4 proj;
	vec3 pos;
} CAMERA;

layout(push_constant) uniform UNIFORM_CONSTANTS
{
	mat4 model;
} CONSTANTS;

void main()
{
	fragUV = UV;
	gl_Position = CAMERA.proj * CAMERA.view * CONSTANTS.model * vec4(VERTEX, 1.0);
}
```

## 예시: 조명 텍스처 셰이더

```glsl
#version 430 core

Shader "Lit Texture"
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
			layout(location = 0) out vec2 fragUV;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormal;

			void main()
			{
				fragUV = UV;
				fragPos = (MATRIX_MODEL * vec4(VERTEX, 1.0)).xyz;
				fragNormal = normalize(mat3(transpose(inverse(MATRIX_MODEL))) * NORMAL);
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * vec4(fragPos, 1.0);
			}
		}

		Stage Fragment
		{
			layout(location = 0) in vec2 fragUV;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormal;
			layout(location = 0) out vec4 outColor;

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
						vec3 toLightVec = LIGHT.lights[i].pos.xyz - fragPos;
						vec3 toLightDir = normalize(toLightVec);
						float lightDis = length(toLightVec);
						float attenuation = clamp(1.0 - (lightDis / LIGHT.lights[i].pos.w), 0.0, 1.0);
						diffuse += max(dot(fragNormal, toLightDir), 0.0) * attenuation;
					}
				}

				outColor = texture(tex, fragUV);
				outColor.rgb *= diffuse + ambient;
			}
		}
	}
}
```

## 예시: 스텐실 아웃라인

첫 번째 패스는 원본 오브젝트를 스텐실에 기록하고 컬러 출력을 막습니다. 두 번째 패스는 노멀 방향으로 정점을 밀어낸 뒤, 스텐실 값이 다른 영역만 칠합니다.

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
		LightingPass "Opaque"
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
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
			}
		}

		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(0.0);
			}
		}
	}

	Pass
	{
		LightingPass "Opaque"
		Cull Back;

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

		Stage Vertex
		{
			uniform float outlineWidth;

			void main()
			{
				vec4 vert = vec4(VERTEX + NORMAL * outlineWidth, 1.0);
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

## 작성 시 주의할 점

- `Stage` 안의 일반 `uniform`은 `Property`에 먼저 선언해야 합니다. 선언이 없으면 파서가 오류를 냅니다.
- `LIGHT`는 `LIGHT.pos[i]`가 아니라 `LIGHT.lights[i].pos` 형태로 접근해야 합니다.
- `MATRIX_MODEL`을 사용하면 push constant가 생성됩니다. Vulkan 구현은 현재 모델 행렬 하나(`mat4`)를 push constant로 사용합니다.
- `MATRIX_VIEW`, `MATRIX_PROJ`, `CAMERA` 중 하나를 사용하면 camera UBO가 생성됩니다.
- `[Local]`은 오브젝트별 데이터를 위한 선언입니다. 같은 머티리얼을 여러 오브젝트가 공유할 때 개별 값을 넣어야 하는 프로퍼티에 사용합니다.
- `sampler2D`는 UBO 멤버가 아니라 별도 sampler descriptor로 생성됩니다.
- `mat2`는 파서와 `Shader` 프로퍼티 타입에는 존재하지만, 현재 `ShaderPass::FillAttributes()`의 UBO 멤버 구성에는 반영되어 있지 않습니다. 머티리얼 프로퍼티로 안정적으로 쓰려면 `mat3` 또는 `mat4`를 사용하거나 해당 경로를 보강해야 합니다.
- 자동 생성되는 UBO 멤버는 최종 레이아웃 최적화를 위해 선언 순서와 다르게 정렬될 수 있습니다. CPU 측 업로드는 `UniformStructLayout`의 offset을 기준으로 하므로 이름으로 접근하는 경로를 사용해야 합니다.
- 패스의 `LightingPass` 이름은 렌더러 쪽에서 조회하는 키입니다. 새 이름을 만들 때는 해당 렌더 단계가 실제로 그 이름을 요청하는지 함께 확인해야 합니다.
