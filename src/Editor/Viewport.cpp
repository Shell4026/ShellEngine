#include "Viewport.h"

#include "Game/ImGUI.h"
#include "Game/World.h"

#include "Render/RenderTexture.h"
#include "Render/VulkanTextureBuffer.h"

namespace sh::editor
{
	Viewport::Viewport(game::ImGUI& imgui, game::World& world, std::mutex& renderMutex) :
		UI(imgui),
		world(world),
		renderMutex(renderMutex),

		viewportDescSet(), renderTex(),
		viewportWidthLast(100.f), viewportHeightLast(100.f),
		bDirty(false)
	{
		renderTex = std::make_unique<render::RenderTexture>();
		renderTex->Build(world.renderer);
		core::GarbageCollection::GetInstance()->SetRootSet(renderTex.get());
		for (int thr = 0; thr < 2; ++thr)
		{
			auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(0));
			auto imgBuffer = vkTexBuffer->GetImageBuffer();

			viewportDescSet[thr] = nullptr;
		}
	}
	Viewport::~Viewport()
	{
		Clean();
	}

	void Viewport::Update()
	{
		//뷰포트 사이즈가 변했을 시 렌더 텍스쳐의 스케일을 바꿔준다.
		std::cout << "changed\n";

		if (viewportDescSet[GAME_THREAD])
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[GAME_THREAD]);
		if (viewportWidthLast != 0.f && viewportHeightLast != 0.f)
			renderTex->SetSize(viewportWidthLast, viewportHeightLast);
		auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(0));
		auto imgBuffer = vkTexBuffer->GetImageBuffer();
		viewportDescSet[GAME_THREAD] = ImGui_ImplVulkan_AddTexture(imgBuffer->GetSampler(), imgBuffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		SetDirty();
	}

	void Viewport::Render()
	{
		ImGui::Begin(name);
		float width = ImGui::GetContentRegionAvail().x;
		float height = ImGui::GetContentRegionAvail().y;
		if (viewportWidthLast != width || viewportHeightLast != height)
		{
			viewportWidthLast = width;
			viewportHeightLast = height;
			if (viewportWidthLast < 0)
				viewportWidthLast = 0;
			if (viewportHeightLast < 0)
				viewportHeightLast = 0;
			Update();
		}
		if (bDirty)
		{
			// 다음 동기화 타이밍 후 viewportDescSet[RENDER_THREAD] = viewportDescSet[GAME_THREAD]가 되기 때문에
			// 드로우 콜에 집어 넣을 때는 미리 GAME_THREAD의 이미지를 넣는다.
			if (viewportDescSet[GAME_THREAD])
				ImGui::Image((ImTextureID)viewportDescSet[GAME_THREAD], { width, height });
		}
		else
		{
			if (viewportDescSet[RENDER_THREAD])
				ImGui::Image((ImTextureID)viewportDescSet[RENDER_THREAD], { width, height });
		}
		ImGui::End();

		imgui.SetDirty();
	}

	auto Viewport::GetRenderTexture() -> render::RenderTexture&
	{
		return *renderTex;
	}

	void Viewport::Clean()
	{
		if (viewportDescSet[GAME_THREAD])
		{
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[GAME_THREAD]);
			viewportDescSet[GAME_THREAD] = nullptr;
		}
		if (viewportDescSet[RENDER_THREAD])
		{
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[RENDER_THREAD]);
			viewportDescSet[RENDER_THREAD] = nullptr;
		}

		renderTex.reset();
	}

	void Viewport::SetDirty()
	{
		if (bDirty)
			return;

		bDirty = true;
		world.renderer.PushSyncObject(*this);
	}
	void Viewport::Sync()
	{
		std::swap(viewportDescSet[RENDER_THREAD], viewportDescSet[GAME_THREAD]);
		bDirty = false;
	}
}//namespace