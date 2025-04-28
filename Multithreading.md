# 개요
ShellEngine은 멀티 스레딩 구조를 가지고 있습니다.

크게 게임 스레드와 렌더 스레드의 구조로 나눠져 있으며, 각 스레드 내에서 분할 가능한 작업을 Task단위로 나눠 스레드 풀에 넣어서 병렬적으로 처리합니다.

![멀티스레드구조](https://github.com/user-attachments/assets/da0f58d0-3a38-4329-8d24-3e3b9007dcfe)

모든 스레드의 작업이 끝난 후 동기화가 시작 되며, 동기화가 끝난 후 쓰레기 수집의 타이밍이 맞다면 쓰레기 수집을 거친 후 스레드들의 작업을 재개합니다.

# 동기화 클래스 설계

```c++
class Texture : 
  public core::SObject, 
  public core::INonCopyable,
  public core::ISyncable
{
  SCLASS(Texture)
/*...*/
    SH_RENDER_API void SyncDirty() override;
    SH_RENDER_API void Sync() override;
};
```
```c++
SH_RENDER_API void Texture::SyncDirty()
{
  if (!bDirty.test_and_set(std::memory_order::memory_order_acquire))
    core::ThreadSyncManager::PushSyncable(*this);
}
SH_RENDER_API void Texture::Sync()
{
  if (bSetDataDirty)
  {
    CreateTextureBuffer();
    bSetDataDirty = false;
  }

  bDirty.clear(std::memory_order::memory_order_relaxed);
}
```
해당 코드는 Texture코드의 일부입니다. 텍스쳐는 여러 스레드에서 접근 할 가능성을 가지며 동기화가 필요합니다. 이러한 객체는 core::ISyncable인터페이스를 상속 후 SyncDirty()와 Sync()를 오버라이딩 하여 구현해야 합니다.

SyncDirty는 텍스쳐의 사이즈가 변경 된 경우와 같이 두 스레드가 공유 해야 할 자원이 변경 됐음을 알리고 core::ThreadSyncManager에 넣는 함수를 구현하면 됩니다.

Sync함수가 동기화 타이밍에 수행 되는 함수이며 이 함수 내부에서 동기화 코드를 작성하면 됩니다.

# 동기화 방식
위의 예시인 텍스쳐 클래스의 동기화 방식은 Sync 타이밍 때 값을 재설정하는 **지연된 업데이트** 방식이라고 부르며, 스레드 별로 변수를 두고 swap 하는 방식인 더블 버퍼링 방식도 쓸 수 있습니다.
```c++
core::SyncArray<uint32_t> drawcall;

SH_RENDER_API void Renderer::Sync()
{
  /*...*/
  std::swap(drawcall[core::ThreadType::Game], drawcall[core::ThreadType::Render]);
}
```
swap방식은 메모리를 더 먹지만 매우 빠르니 필요한 곳에 쓰면 됩니다.
