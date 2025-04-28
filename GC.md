# 개요
ShellEngine은 리플렉션 시스템을 이용해 마크 앤 스윕 가비지 컬렉터를 구현했습니다.

모든 SObject객체는 가비지 컬렉터의 추적을 받으며 RootSet에 등록된 객체부터 시작하여 마킹을 시작합니다. 마킹이 되지 않은 객체는 제거 됩니다.

RootSet 수가 많아지면 가비지 컬렉터는 스레드 풀을 활용해 병렬적으로 잠금 없이 빠르게 마킹을 수행합니다.

# 객체 유효성 검사
core/Util.h에 존재하는 bool IsValid(const SObject* obj); 함수를 이용해 객체가 제거 될 객체인지, nullptr인지 검증 할 수 있습니다.

```c++
for (auto& obj : objs)
{
  if (!sh::core::IsValid(obj))
    continue;
  if (!obj->activeSelf)
    continue;
  obj->Update();
}
```
# 객체 추적
클래스에서 특정 SObject 객체의 포인터를 가지고 있고, 이 포인터를 PROPERTY매크로를 통해 리플렉션에 노출 시키면 가비지 컬렉터는 해당 객체가 사용 되고 있다는 것을 알게 됩니다.

```c++
PROPERTY(mesh)
const Mesh* mesh;
PROPERTY(mat)
Material* mat;
PROPERTY(drawable, core::PropertyOption::invisible, core::PropertyOption::noSave)
Drawable* drawable;
```
해당 포인터가 가르키고 있는 객체가 지워진다면 가비지 컬렉터에서 해당 포인터를 nullptr로 바꾸므로 댕글링 포인터를 방지 할 수 있습니다.

또한 core/Scontainer.hpp에 존재하는 컨테이너들을 이용한다면 프로퍼티에 등록 할 필요 없이 객체를 추적 할 수 있습니다. </br>
Vector와 Array객체와 같은 요소의 삭제가 느린 컨테이너의 요소가 제거 되면 nullptr로 바뀌며, 삭제가 빠른 자료구조들은 객체가 제거(erase) 됩니다.
```c++
sh::core::SHashSet<GameObject*> objHashSet;
sh::core::SVector<GameObject*> objVector;
sh::core::SMap<GameObject*, int> objMap;
```

특정 객체를 RootSet으로 지정하는 방식은 다음 코드처럼 하면 됩니다.
```c++
gc.SetRootSet(objPtr);
```

# 내부 흐름
가비지 컬렉터는 동기화 타이밍 이후, 스레드들이 깨어나기 전 작동합니다. 이로 인해 멀티스레딩 환경에서도 안전하게 사용 할 수 있습니다.

```c++
ProcessInput();

world->Update(window->GetDeltaTime());

core::ThreadSyncManager::Sync();
gc->Update();
world->AfterSync();

core::ThreadSyncManager::AwakeThread();
```
