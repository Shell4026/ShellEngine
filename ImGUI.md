# ImGUI의 멀티 스레드 지원

ImGUI라이브러리는 기본적으로 싱글 스레드에서 작동하게 설계 됐습니다.

ShellEngine에서는 이를 해결 하기 위해 게임 스레드에서 ImGui::~ 함수로 만들어지는 ImDrawList의 데이터를 Sync타이밍에 별도의 ImDrawList로 옮긴 후, 렌더 스레드에서 렌더링 하는 구조로 변경하였습니다.

즉, ImDrawList는 게임 스레드에서 Sync타이밍을 거쳐 렌더 스레드로 전달 됩니다.
```c++
	void ImGUImpl::Sync()
	{
		ImDrawData* src = ImGui::GetDrawData();
		if (src == nullptr || !src->Valid)
			return;

		ClearDrawData();

		// CmdLists 제외 복사
		ImVector<ImDrawList*> temp;
		temp.swap(src->CmdLists);
		drawData = *src;
		temp.swap(src->CmdLists);

		// CmdLists 요소 복사
		drawData.CmdLists.resize(src->CmdLists.Size);
		for (int i = 0; i < drawData.CmdListsCount; ++i)
		{
			ImDrawList* copy = src->CmdLists[i]->CloneOutput();
			drawData.CmdLists[i] = copy;
		}

		bDirty = false;
	}
```
이렇게 복사한 ImDrawList는 UIPass에서 이용해 렌더 스레드에서 그려집니다.
```c++
SH_GAME_API auto UIPass::BuildDrawList(const render::RenderTarget& renderData) -> render::DrawList
{
  render::DrawList list{};

  list.drawCall.push_back(
    [&](render::CommandBuffer& cmd)
    {
      ImGui_ImplVulkan_RenderDrawData(&gui->GetDrawData(), static_cast<render::vk::VulkanCommandBuffer&>(cmd).GetCommandBuffer());
    }
  );
  list.bClearColor = false;

  return list;
}
```

만일 sync타이밍에 imgui에서 쓰인 텍스쳐가 변동이 된다면 ImDrawList내부의 ImDrawCmd를 직접 변경해 줘야 합니다.

이 과정은 복잡하여 자동으로 처리 해주는 GUITexture클래스를 사용하면 편리합니다.

```c++
renderTex = world.GetGameObject("EditorCamera")->GetComponent<game::EditorCamera>()->GetRenderTexture();
viewportTexture = core::SObject::Create<game::GUITexture>();
viewportTexture->Create(*renderTex);

ImGui::Begin(name);
...
if (viewportTexture->IsValid())
	viewportTexture->Draw(ImVec2{ width, height }); // ImGui::Image()와 같다.
...
ImGui::End();
```
