# 개요
https://github.com/user-attachments/assets/1a99934c-ab8c-4e8b-91c5-620a6b393010

[reactphyscis3d](https://github.com/DanielChappuis/reactphysics3d)라이브러리를 사용하여 엔진에서 물리 효과를 처리합니다.</br>
아직 래퍼 클래스가 부족한 상태라 더 많은 시간이 필요합니다.

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
