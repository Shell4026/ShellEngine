# 개요
https://github.com/user-attachments/assets/1a99934c-ab8c-4e8b-91c5-620a6b393010

https://github.com/user-attachments/assets/583008aa-64ec-46ff-8cfa-820567d29e2f

[reactphyscis3d](https://github.com/DanielChappuis/reactphysics3d)라이브러리를 사용하여 엔진에서 물리 효과를 처리합니다.</br>

물리 로직의 업데이트는 다른 업데이트 함수들과 달리 고정된 업데이트 주기(2ms)를 가져 현재 프레임에 관계 없이 일정한 움직임을 보여줍니다.
```c++
_fixedDeltaTime += _deltaTime;
while (_fixedDeltaTime >= FIXED_TIME)
{
  physWorld.Update(FIXED_TIME);
  for (auto& obj : objs)
  {
    if (!sh::core::IsValid(obj))
      continue;
    if (!obj->activeSelf)
      continue;
    obj->FixedUpdate();
  }
  _fixedDeltaTime -= FIXED_TIME;
}
```
## 충돌 체크
```c++
void OnCollisionEnter(Collider& collider);
void OnCollisionStay(Collider& collider);
void OnCollisionExit(Collider& collider);
void OnTriggerEnter(Collider& collider);
void OnTriggerStay(Collider& collider);
void OnTriggerExit(Collider& collider);
```
해당 함수들을 override한 후 rigidbody가 붙어 있는 게임 오브젝트에 컴포넌트를 추가하면 충돌 신호를 받을 수 있습니다.
