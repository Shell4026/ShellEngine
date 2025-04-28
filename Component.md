# 개요
유니티와 유사한 컴포넌트 시스템입니다. sh::game::component 클래스를 상속 후 COMPONENT매크로를 통해 컴포넌트로 등록합니다.

PROPERTY매크로를 변수 위에 두면 에디터내에서 노출됩니다.

리플렉션 시스템과 직렬화-역직렬화를 통해 유저가 짠 코드를 즉각적으로 볼 수 있는 핫-로드 기능을 지원합니다.

## 핫로드 영상
https://github.com/user-attachments/assets/41223f10-49be-48cd-997d-c135991a62fa

# 예시 코드
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


