# ShellEngine
![스크린샷 2024-10-26 212736](https://github.com/user-attachments/assets/8f0001a3-71a0-4e79-9854-9e41714507eb)</br>
Vulkan C++17 크로스 플랫폼 게임 엔진 프로젝트

# 프로젝트 구조

![흐름도](https://github.com/user-attachments/assets/79eef4d4-5b85-4093-8597-183433164c18)
> 멀티 스레딩 구조로, 게임 스레드와 렌더 스레드로 나눠져 있습니다. </br>
> 각 스레드는 작업이 끝난 후 Sync타이밍에 버퍼를 교환하며 가비지 컬렉터가 작동합니다.

## 구성 요소
![구조](https://github.com/user-attachments/assets/2cbb3291-e7cd-4441-86dc-6fe32df651c6)

### Core 모듈
> 하위 모듈에서 쓸 공통적인 코드와 핵심 기능을 정의해둔 모듈입니다. </br>
> 리플렉션과 GC 기능이 핵심입니다. </br>
> [Reflection.hpp](https://github.com/Shell4026/ShellEngine/blob/main/include/Core/Reflection.hpp) [GarbageCollection.cpp](https://github.com/Shell4026/ShellEngine/blob/main/src/Core/GarbageCollection.cpp)</br>
### Window 모듈
> Windows와 Linux의 창을 생성하고 OS의 이벤트를 받는 모듈입니다.
### Render 모듈
> 렌더링에 필요한 기능들을 담고 있는 모듈입니다. </br>
> 현재는 Vulkan API만 지원하며 추후 확장을 고려해 설계했습니다. </br>
> 큐에 IDrawable 객체를 받아 렌더링 하는 구조입니다. [VulkanRenderer.cpp](https://github.com/Shell4026/ShellEngine/blob/main/src/Render/VulkanImpl/VulkanRenderer.cpp)
### Physics 모듈
> [ReactPhysics3D](https://www.reactphysics3d.com/)라이브러리를 사용한 물리 모듈입니다.
### Game 모듈
> 실제 게임에서 사용되는 객체와 컴포넌트들이 정의된 모듈입니다.</br>
> COMPONENT 매크로를 사용하고 Component객체를 상속해 컴포넌트를 정의합니다.</br>
> [GameObject.h](https://github.com/Shell4026/ShellEngine/blob/main/include/Game/GameObject.h) [Component.h](https://github.com/Shell4026/ShellEngine/blob/main/include/Game/Component/Component.h)
### User 모듈
> 사용자가 짜는 코드는 동적 라이브러리 형태로 이 모듈에 포함되며, 엔진이 시작 될 때 불러옵니다. </br>
### Editor 모듈
> 에디터에서 사용하는 코드와 기능을 담고 있는 모듈입니다. </br>
> 리플렉션을 사용해 객체의 정보를 표시합니다. </br>
> [Hierarchy.cpp](https://github.com/Shell4026/ShellEngine/blob/main/src/Editor/Hierarchy.cpp) [Inspector.cpp](https://github.com/Shell4026/ShellEngine/blob/main/src/Editor/Inspector.cpp)

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
