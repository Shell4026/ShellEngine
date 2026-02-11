# 개요
CustomInspector을 상속 받고 INSPECTOR매크로를 통해 사용자 정의 Inspector를 보여줄 수 있습니다.

RenderUI함수를 오버라이딩한 후 그 안에 ImGUI:: 함수를 호출하면 됩니다.

```c++
INSPECTOR(클래스, Inspector에서 보여줄 클래스)
// obj 표시될 인스턴스의 포인터
// idx Inspector내에서 몇번째 인스턴스인지
void RenderUI(void* obj, int idx);
```

> [!Important]
> 에디터 관련 클래스들은 프로젝트 폴더의 EditorSource내에서 만들고 작업해야 합니다.</br>
> RenderUI내에서 상속 받은 CustomInspector의 RenderUI를 반드시 호출해줘야 합니다.

## 예시 코드
```cpp
namespace sh::editor
{
	class RotateObjectInspector : public CustomInspector
	{
		INSPECTOR(RotateObjectInspector, game::RotateObject) // RotateObject라는 사용자 정의 컴포넌트
	public:
		SH_EDIT_API void RenderUI(void* obj, int idx) override;
	};
}//namespace
```
```cpp
namespace sh::editor
{
	SH_EDIT_API void RotateObjectInspector::RenderUI(void* obj, int idx)
	{
		CustomInspector::RednerUI(obj, idx); // 중요
		
		game::RotateObject* rotateObjPtr = reinterpret_cast<game::RotateObject*>(obj);
		ImGui::SetCurrentContext(rotateObjPtr->gameObject.world.GetUiContext().GetContext()); // 필수
		ImGui::Text("This is RotateObject!!!");
		Inspector::RenderProperties(rotateObjPtr->GetType(), *rotateObjPtr, idx); // 기본 프로퍼티 렌더링 방법
	}
}//namespace
```
## 결과
<img width="203" height="522" alt="image" src="https://github.com/user-attachments/assets/0c67ec08-2ec5-4d42-90dc-b7db09cbb83b" />
