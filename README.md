# ShellEngine
![image](https://github.com/user-attachments/assets/c5aa3796-8969-4b63-aee8-8c236fbe6820)

Vulkan 기반으로 개발중인 크로스 플랫폼 3D 게임 엔진입니다.</br>
저수준 그래픽 API를 직접 다루면서, 현대 게임 엔진에 필요한 다양한 시스템을 설계하고 구현하는 것을 목표로 했습니다.

## 특징
- **Vulkan 기반 렌더링 시스템**  
  - 로우레벨 Vulkan API를 직접 사용하여 고성능 그래픽 파이프라인 구현
  - 추후 확장을 고려한 렌더러 추상화 구조 설계

- **멀티스레딩 아키텍처** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Multithreading.md)
  - 게임 스레드와 렌더 스레드 분리
  - **이중 버퍼링**, **지연 동기화** 구조를 통한 프레임 간 안전한 데이터 교환
  - 한 프레임 후 Sync 타이밍에 맞춰 가비지 컬렉션 및 메모리 정리 수행
  - 나눌 수 있는 작업은 Task로 분리 후 스레드 풀에 넣어 병렬적으로 수행
  - Vulkan 커맨드 버퍼 병렬 기록

- **런타임 리플렉션 시스템** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Reflection.md)  
  - 객체의 메타데이터를 런타임에 조회 및 수정 가능
  - 이를 기반으로 에디터 기능, **가비지 컬렉터** 구현

- **간단한 메모리 관리** [상세](https://github.com/Shell4026/ShellEngine/blob/main/GC.md)
  - 리플렉션 기반 마크 앤 스윕 가비지 컬렉터를 통한 누수 없고 쉬운 메모리 관리
  - PROPERTY매크로와 쓰레기 수집을 통한 댕글링 포인터 방지
  - STL 컨테이너를 상속한 가비지 컬렉터용 컨테이너

- **에디터 통합 개발** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Editor.md)
  - 런타임 리플렉션을 활용한 오브젝트 계층 구조(Hierarchy) 및 속성 편집(Inspector) 구현
  - Unity 스타일의 편집 환경 제공
  - 객체 조작 방식은 Blender와 유사

- **컴포넌트 기반 아키텍처** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Component.md)
  - Unity와 유사한 컴포넌트-엔티티 구조
  - 사용자가 작성한 컴포넌트를 DLL로 **핫로드**하여 런타임 중 추가 및 수정 가능

- **간단한 셰이더 언어와 파서** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Shader.md)
  - 유니티 ShaderLab과 유사한 문법을 가진 셰이더 언어
  - 하향식 파서를 구현하여 파이프라인 설정을 셰이더 코드에서 할 수 있음

- **Physics 시스템 통합** [상세](https://github.com/Shell4026/ShellEngine/blob/main/Physics.md)
  - ReactPhysics3D를 연동하여 물리 연산 및 충돌 처리를 지원

- **크로스 플랫폼 지원 (Windows/Linux)**  
  - Windows 및 Linux 환경에서 빌드 및 실행 가능
  - 플랫폼 독립적인 창 생성 및 이벤트 처리 모듈화
  - Windows: win32 api, Linux: x11 lib

## 간단한 데모
https://github.com/user-attachments/assets/50c5cc05-7eba-45cf-9a92-ca2046e119d2

## 프로젝트 구조 흐름

![흐름도](https://github.com/user-attachments/assets/79eef4d4-5b85-4093-8597-183433164c18)
> 멀티 스레딩 구조로, 게임 스레드와 렌더 스레드로 나눠져 있습니다. </br>
> 각 스레드는 작업이 끝난 후 Sync타이밍에 버퍼를 교환하며 가비지 컬렉터가 작동합니다.
> 자세한 사항은 '특징'의 멀티 스레딩 아키텍쳐를 참조 하세요.

![생명주기 drawio](https://github.com/user-attachments/assets/ffee5ba7-44ab-4733-bbaf-6d486e343fe0)

게임 스레드내의 모든 객체는 해당 생명 주기를 따릅니다.

## 구성 요소
![구조](https://github.com/user-attachments/assets/2cbb3291-e7cd-4441-86dc-6fe32df651c6)

# 설치
## Windows

**Required**

VulkanSDK : https://vulkan.lunarg.com/sdk/home#windows

## Linux

**Required**
```
sudo apt install build-essential ninja-build
```
```
sudo apt install libx11-dev libxext-dev libgl1-mesa-dev
```
VulkanSDK : https://vulkan.lunarg.com/sdk/home#linux

# TODO
- 스켈레탈 메쉬
- 애니메이션, 애니메이터
