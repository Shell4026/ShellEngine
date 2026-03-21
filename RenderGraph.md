# ShellEngine 렌더 그래프 기반 렌더링 구조

## 개요

ShellEngine의 Vulkan 렌더링 시스템은 드로우 호출을 단일 흐름으로 나열하는 방식 대신, **패스 단위로 렌더링 작업을 분리**하고 각 패스가 사용하는 렌더 텍스처의 읽기/쓰기 관계를 추적하여, 리소스 전이와 실행 흐름을 자동으로 조립하는 구조로 설계되어 있습니다.

이 구조는 `ScriptableRenderPass`, `ScriptableRenderer`, 그리고 Vulkan 구현 계층이 역할을 나누어 담당합니다.

해당 구조로 인해 프로젝트 목적에 맞게 코드를 크게 수정하지 않고 패스를 재배치 할 수 있습니다.

---

## 설계 배경

Vulkan 렌더링 시스템을 구현할 때 드로우 호출 자체보다 **리소스 상태 관리와 실행 순서**가 더 큰 문제가 됩니다. 구체적으로 아래 사항들을 매 패스마다 직접 추적해야 합니다.

- 패스 간 실행 순서
- 각 패스의 렌더 타겟 및 샘플링 텍스처
- 이미지 레이아웃 및 접근 권한 전이 타이밍
- `vkCmdPipelineBarrier` 삽입 위치

렌더링 코드를 단일 흐름으로 구성할 경우, 패스가 늘어날수록 다음과 같은 문제가 발생합니다.

- 패스 추가 시 앞뒤 리소스 상태 전이를 전체적으로 재검토해야 함
- 배리어 누락 또는 잘못된 레이아웃 전이로 인한 런타임 오류 발생
- 렌더링 의도 코드와 Vulkan 동기화 코드가 강하게 결합됨

ShellEngine은 이 문제를 해결하기 위해, 그리고 유연한 확장을 위해 패스의 렌더링 의도와 API 레벨의 전이 처리를 분리하는 구조를 채택했습니다.

---

## 설계 목표

| 계층 | 책임 |
|---|---|
| `ScriptableRenderPass` | 렌더링 의도 선언 (타겟, 샘플링 텍스처, 드로우 목록) |
| `ScriptableRenderer` | 패스 간 리소스 상태 추적 및 전이 정보 계산 |
| Vulkan 구현 계층 | 전이 정보를 실제 Vulkan API 호출로 번역 |

패스는 자신의 렌더링 의도만 기술하며, 배리어 생성 및 이미지 레이아웃 관리는 엔진이 대신 수행합니다.

---

## 핵심 구성 요소

### ScriptableRenderPass

렌더링을 구성하는 기본 단위입니다. 각 패스는 다음 정보를 가집니다.

| 필드 | 설명 |
|---|---|
| `passName` | 패스 식별자 |
| `renderQueue` | 실행 순서 |
| `drawList` | 드로우 목록 |
| `renderTextures` | 사용 리소스 및 용도 |

`renderTextures`에는 패스가 사용하는 렌더 텍스처와 그 사용 방식이 기록됩니다.

| 사용 방식 | 의미 |
|---|---|
| `ColorAttachment` | 렌더 타겟 (출력) |
| `SampledRead` | 샘플링 텍스처 (입력) |
| `Src` | 복사 소스 |
| `Present` | 스왑체인 출력 |

패스가 '이 리소스를 이 용도로 사용한다'는 사실을 명시적으로 드러냄으로써, 렌더러가 전체 흐름을 계산하는 기반이 됩니다.

---

### Configure 단계

`ScriptableRenderPass::Configure()`는 렌더러가 전이를 계산하는 데 필요한 데이터를 구성하는 단계입니다.

1. `BuildDrawList()` - 이번 패스의 드로우 목록 생성
2. `CollectRenderImages()` - 렌더 텍스처 수집 및 용도 분류
   - 현재 렌더 타겟 -> `ColorAttachment`
   - DrawList로 전달 된 메테리얼이 참조하는 `RenderTexture` -> `SampledRead`
3. 동일 패스 내에서 같은 텍스처를 출력과 입력으로 동시에 사용하는 경우 검증

이 단계를 통해 패스의 리소스 사용 관계가 확정됩니다.

> [!Note]
> ```BuildDrawList```에는 RenderTarget구조체가 전달 됩니다.
> ```cpp
> struct RenderTarget
> {
>	uint32_t frameIndex;
>	const Camera* camera;
>	const RenderTexture* target;
>	const std::vector<Drawable*>* drawables;
> };
> ```
---

### ScriptableRenderer

`ScriptableRenderer::Execute()`는 패스 간 리소스 연결 관계를 계산하고 커맨드 버퍼 기록을 조율합니다.

1. 활성 패스 수집
2. renderQueue 기준 정렬
3. 각 패스 Configure() 호출 -> 리소스 사용 정보 확정
4. 각 패스 BuildBarrierInfo() 호출 -> 전이 정보 계산
5. 커맨드 버퍼 기록

패스는 직접 배리어를 생성하지 않으며, 리소스 사용 의도만 제공합니다. 실제 전이 계산은 렌더러가 전담합니다.

---

### BarrierInfo

`BarrierInfo`는 패스 간 리소스 의존성을 실행 가능한 형태로 담는 구조체입니다.

| 필드 | 설명 |
|---|---|
| 대상 이미지 | 전이 대상 리소스 |
| `lastUsage` | 이전 패스의 사용 상태 |
| `curUsage` | 현재 패스의 사용 상태 |

렌더러는 각 텍스처의 사용 이력을 추적하여 패스 사이의 상태 전이를 `BarrierInfo`로 구체화합니다.

**예시 - 3패스 렌더링 흐름:**

```
패스 1: 씬 렌더링     ->  Undefined         -> ColorAttachment
패스 2: 후처리 샘플링 ->  ColorAttachment   -> SampledRead
패스 3: 스왑체인 출력 ->  SampledRead       -> Present
```

---

### Vulkan 구현 계층

`VulkanRenderImpl::EmitBarrier()`는 렌더러가 계산한 `BarrierInfo`를 Vulkan API 호출로 번역합니다.

- `ImageUsage`를 image layout, pipeline stage, access mask로 매핑
- `VulkanImageBuffer::BarrierCommand()`를 통해 `vkCmdPipelineBarrier` 기록

이 계층은 전이가 필요한지 판단을 내리지 않으며, 상위에서 계산된 전이 정보를 API 호출로 변환하는 역할만 수행합니다.

---

## 병렬 커맨드 버퍼 기록

이 구조는 멀티스레딩과도 자연스럽게 연결됩니다.

`ScriptableRenderer::Execute()`는 다음 두 단계로 분리됩니다.

1. **직렬 단계** — 중앙에서 리소스 상태 정리 및 `BarrierInfo` 계산
2. **병렬 단계** — 각 패스의 커맨드 버퍼 기록을 스레드 풀에 task로 분배하여 병렬 실행

리소스 전이 판단이 중앙에서 선행되므로, 이후 커맨드 기록 단계는 패스 간 의존성 없이 병렬화할 수 있습니다.

---

## 패스 추가 방법

새 패스를 추가할 때 필요한 작업은 다음으로 제한됩니다.

1. `ScriptableRenderPass` 구현체 작성
2. `renderQueue` 설정
3. 필요 시 ```BuildDrawList``` 오버라이딩 하여 드로우 할 물체 필터링
4. `Configure()`에서 드로우 목록 및 리소스 등록
5. `Record()`에서 커맨드 기록

기존 패스나 배리어 코드를 수정할 필요가 없습니다. `CopyPass`처럼 드로우와 성격이 다른 패스도 동일한 체계 안에서 전이 계산을 공유합니다.

---

## 한계

현재 구조는 아래 기능은 포함되어 있지 않습니다.

- 의존성 기반 패스 자동 재배치
- 버퍼 리소스 상태 추적 (현재는 `ImageUsage` 위주로 구현됨)

---

## 구조 요약

ScriptableRenderPass (N개)
- renderTextures: 리소스 사용 의도 선언

ScriptableRenderer
- 패스 순서 정렬
- BuildBarrierInfo(): 패스 간 상태 전이 계산
- 커맨드 버퍼 병렬 기록

VulkanRenderImpl
- EmitBarrier(): ImageUsage -> Vulkan barrier 변환
- vkCmdPipelineBarrier 기록
