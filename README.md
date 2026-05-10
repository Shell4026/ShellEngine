# ShellEngine
<img width="1262" height="816" alt="image" src="https://github.com/user-attachments/assets/4f0cca0c-df1f-487c-97f7-94a8eccc1ad3" />

<img width="1010" height="761" alt="스크린샷 2026-05-11 012031" src="https://github.com/user-attachments/assets/d51c882d-c52a-40da-8002-fb01c0d275a2" />


[![Video Label](http://img.youtube.com/vi/SEiktv0WtOM/0.jpg)](https://youtu.be/SEiktv0WtOM)</br>
[간단한 에디터 데모 영상(Youtube)](https://youtu.be/SEiktv0WtOM)

## 개요
저수준 그래픽 API를 직접 다루면서 현대 게임 엔진의 핵심 시스템을 설계, 구현하는 것을 목표로 한 1인 개발 프로젝트입니다.

렌더링 파이프라인 / 멀티스레딩 아키텍처 / 메모리 관리 / 에디터 통합 영역을 스스로 설계했습니다.

## 특징 및 문서
### Vulkan 기반 렌더링 시스템 | [상세 문서](RenderArchitecture.md)
로우레벨 Vulkan API를 직접 사용하여 고성능 그래픽 파이프라인을 구현했습니다.</br>
추후 패스 추가 및 구조 변경이 쉽도록 유니티의 SRP와 유사한 패스 기반 구조를 채택했습니다.
- Vulkan 커맨드 버퍼 병렬 기록
---
### 멀티스레딩 아키텍처 | [상세 문서](Multithreading.md)
게임 스레드와 렌더 스레드를 분리하여 CPU와 GPU 작업이 병렬로 진행되도록 설계했습니다.
- 이중 버퍼링 + 지연 동기화: 두 스레드가 서로 블로킹하지 않고 프레임 간 안전하게 데이터를 교환
- 가비지 컬렉션 타이밍: Sync 타이밍에 맞춰 GC가 실행되어 렌더 중 메모리 해제로 인한 크래시를 방지
- 스레드 풀: 분리 가능한 작업은 Task로 쪼개 병렬 처리, Vulkan 커맨드 버퍼도 병렬로 기록
- ImGUI 멀티스레드 지원: 기본적으로 멀티스레드를 지원하지 않는 ImGUI를 수정하여 통합 | [상세 문서](https://github.com/Shell4026/ShellEngine/blob/main/ImGUI.md)
---
### 런타임 리플렉션 시스템 | [상세 문서](Reflection.md) | [구현 과정 (velog)](https://velog.io/@shell4026/ShellEngine-%EB%A6%AC%ED%94%8C%EB%A0%89%EC%85%98-%EC%8B%9C%EC%8A%A4%ED%85%9C)
C++은 기본적으로 런타임 리플렉션을 지원하지 않아 직접 설계, 구현했습니다.
- 객체의 타입 정보와 멤버 변수 메타데이터를 런타임에 조회, 수정 가능
- 이를 기반으로 에디터의 Inspector 자동 생성과 가비지 컬렉터를 구현 - 리플렉션 하나로 여러 시스템이 구동되는 구조
---
### 리플렉션 기반 가비지 컬렉터 | [상세 문서](GC.md) | [구현 과정 (velog)](https://velog.io/@shell4026/ShellEngine-GC%EB%A5%BC-%EB%A7%8C%EB%93%A4%EA%B8%B0%EA%B9%8C%EC%A7%80)
C++에서 메모리 관리를 단순화하기 위해 마크 앤 스윕 방식의 GC를 직접 구현했습니다.
- ```PROPERTY``` 매크로로 등록된 포인터를 리플렉션으로 추적하여 댕글링 포인터를 자동으로 nullptr 처리
- STL 컨테이너를 상속한 GC 전용 컨테이너로 컨테이너 내 포인터도 안전하게 관리
- 멀티스레딩 구조의 Sync 타이밍에 동작하여 렌더 중 해제로 인한 문제를 원천 차단
- ```GCObject```를 상속하여 사용자 정의 구조체 내부의 ```SObject```도 추적 가능
---
### 에디터 [상세 문서](Editor.md) | [구현 과정 (velog)](https://velog.io/@shell4026/%EC%97%90%EB%94%94%ED%84%B0-%EA%B0%9C%EB%B0%9C-%EA%B3%BC%EC%A0%95)
- 런타임 리플렉션을 활용한 Hierarchy(계층 구조) 및 Inspector(속성 편집) 자동 생성
- 사용자 정의 Inspector 지원 | [상세 문서](https://github.com/Shell4026/ShellEngine/blob/main/CustomInspector.md)
- 유저 코드 핫-리로드: 엔진 재시작 없이 런타임 중 컴포넌트 코드 추가, 수정 가능 | [구현 과정 (velog)](https://velog.io/@shell4026/ShellEngine-%ED%95%AB-%EB%A6%AC%EB%A1%9C%EB%93%9C-%EA%B5%AC%ED%98%84-%EA%B3%BC%EC%A0%95)
- Unity 스타일의 편집 환경 / Blender 스타일의 오브젝트 조작 방식
- 빌드 시스템
---
### 커스텀 셰이더 언어 | [상세 문서](Shader.md) | [구현 과정 (velog)](https://velog.io/@shell4026/%EC%85%B0%EC%9D%B4%EB%8D%94-%ED%8C%8C%EC%84%9C-%EA%B0%9C%EB%B0%9C-%EA%B3%BC%EC%A0%95)
Unity ShaderLab과 유사한 문법의 셰이더 언어와 하향식 파서(Top-Down Parser) 를 직접 구현했습니다.</br>
파이프라인 설정(블렌딩, 컬링 등)을 셰이더 코드 안에서 선언적으로 작성할 수 있습니다.

---
### 컴포넌트 기반 아키텍처 | [상세 문서](Component.md)
Unity와 유사한 컴포넌트-엔티티 구조를 채택하여 기능의 조합과 확장이 용이하도록 설계했습니다.

---
### Physics 시스템 | [상세 문서](Physics.md)
ReactPhysics3D를 연동하여 물리 연산 및 충돌 처리를 지원합니다.

---
### 크로스 플랫폼 지원 (Windows / Linux)
- Windows: Win32 API / Linux: X11
- 플랫폼 독립적인 창 생성 및 이벤트 처리 모듈화
  
## 프로젝트 구조 흐름

![흐름도](https://github.com/user-attachments/assets/79eef4d4-5b85-4093-8597-183433164c18)
> [!NOTE]
> 게임 스레드와 렌더 스레드가 분리되어 동작하며, Sync 타이밍에 버퍼를 교환하고 GC가 실행됩니다.
> 자세한 사항은 '특징 및 문'의 멀티 스레딩 아키텍쳐를 참조 하세요.

<img width="379" height="731" alt="생명주기 drawio" src="https://github.com/user-attachments/assets/5bf4d552-2ee7-44bf-9cd1-92d051f8fee2" />

게임 스레드내의 모든 객체는  생명 주기를 따릅니다.

## 구성 요소
![구조](https://github.com/user-attachments/assets/2cbb3291-e7cd-4441-86dc-6fe32df651c6)

## 설치
### Windows

**Required**
- VulkanSDK : https://vulkan.lunarg.com/sdk/home#windows
- Visual Studio C++ 빌드 도구(MSVC, cl.exe)
- CMake
- Ninja

디버그 빌드
```powershell
cmake --preset x64-debug
cmake --build out/build/x64-debug
```
릴리즈 빌드
```powershell
cmake --preset x64-release
cmake --build out/build/x64-release
```

또는
해당 저장소를 다운 받은 후 Visual Studio에서 폴더를 열고 CMake로 빌드

### Linux

**Required**
- VulkanSDK : https://vulkan.lunarg.com/sdk/home#linux
```
sudo apt install build-essential ninja-build
sudo apt install libx11-dev libxext-dev libgl1-mesa-dev
```

디버그 빌드
```
cmake --preset linux-debug
cmake --build out/build/linux-debug
```

릴리즈 빌드
```
cmake --preset linux-release
cmake --build out/build/linux-release
```

# TODO
- [x] 스켈레탈 메쉬
- [ ] 애니메이션, 애니메이터
- [x] Undo/Redo
