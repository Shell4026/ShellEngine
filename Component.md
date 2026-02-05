# 개요
유니티와 유사한 컴포넌트 시스템입니다. sh::game::component 클래스를 상속 후 COMPONENT매크로를 통해 컴포넌트로 등록합니다.

PROPERTY매크로를 변수 위에 두면 에디터내에서 노출됩니다.

리플렉션 시스템과 직렬화-역직렬화를 통해 유저가 짠 코드를 즉각적으로 볼 수 있는 핫-로드 기능을 지원합니다.

## 핫로드 영상
https://github.com/user-attachments/assets/41223f10-49be-48cd-997d-c135991a62fa

# 코드 작성법
윈도우 기준 Visual studio로 프로젝트 폴더내에 Source폴더를 열면 CMake를 통해 개발 환경이 세팅 됩니다.

Source폴더내에서 컴포넌트 코드를 작성하고 빌드를 하면 bin폴더에 ShellEngineUser.dll이 생성됩니다.
해당 dll을 엔진에서 불러와 컴포넌트를 로드합니다.

EditorSource폴더내에서는 커스텀 Inspector코드를 작성 할 수 있습니다.

## 컴포넌트 예시 코드
[헤더]
```c++
#pragma once
#include "Export.h"

#include "Game/Component/Component.h"

class RotateObject : public sh::game::Component
{
	COMPONENT(RotateObject, "user")
private:
	PROPERTY(speed)
	float speed;
	PROPERTY(xspeed)
	float xspeed = 0;
	PROPERTY(zspeed)
	float zspeed = 0;
public:
	SH_USER_API RotateObject(sh::game::GameObject& owner);
	SH_USER_API ~RotateObject();

	SH_USER_API void OnEnable() override;
	SH_USER_API void Update() override;
};
```
[cpp]

```c++
SH_USER_API void RotateObject::Update()
{
	Vec3 rot = gameObject.transform->rotation;
	gameObject.transform->SetRotation(rot + Vec3{ xspeed * world.deltaTime, speed * world.deltaTime, zspeed * world.deltaTime });
}
```

[결과]

https://github.com/user-attachments/assets/333185c3-021d-49ae-b09d-198a3cfa1e04

## 커스텀 Inspector 코드 예시
[헤더]
```c++
#pragma once
#include "ExportEditor.h"
#include "RotateObject.h"

#include "Editor/UI/CustomInspector.h"

namespace sh::editor
{
	class RotateObjectInspector : public ICustomInspector
	{
		INSPECTOR(RotateObjectInspector, game::RotateObject)
	public:
		SH_EDIT_API void RenderUI(void* obj, int idx) override;
	};
}//namespace
```
[cpp]
```c++
#include "RotateObjectInspector.h"

#include "Editor/UI/Inspector.h"

#include "Game/ImGUImpl.h"
#include "Game/World.h"
namespace sh::editor
{
	SH_EDIT_API void RotateObjectInspector::RenderUI(void* obj, int idx)
	{
		game::RotateObject* rotateObjPtr = reinterpret_cast<game::RotateObject*>(obj);
		ImGui::SetCurrentContext(rotateObjPtr->gameObject.world.GetUiContext().GetContext()); // 필수
		ImGui::Text("This is RotateObject!!!");
		Inspector::RenderProperties(rotateObjPtr->GetType(), *rotateObjPtr, idx); // 기본 프로퍼티 렌더링 방법
	}
}//namespace
```
[결과]

<img width="203" height="522" alt="image" src="https://github.com/user-attachments/assets/0c67ec08-2ec5-4d42-90dc-b7db09cbb83b" />
