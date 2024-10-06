#include "Viewport.h"

#include "Core/Logger.h"

#include "Game/ImGUImpl.h"
#include "Game/World.h"
#include "Game/Input.h"
#include "Game/Component/Camera.h"
#include "Game/GameObject.h"

#include "Render/RenderTexture.h"
#include "Render/VulkanImpl/VulkanTextureBuffer.h"

namespace sh::editor
{
	Viewport::Viewport(game::ImGUImpl& imgui, game::World& world) :
		UI(imgui),
		world(world),

		viewportDescSet(), renderTex(),
		viewportWidthLast(100.f), viewportHeightLast(100.f),
		bDirty(false)
	{
		renderTex = core::SObject::Create<render::RenderTexture>();
		renderTex->Build(world.renderer);
		core::GarbageCollection::GetInstance()->SetRootSet(renderTex);

		for (int thr = 0; thr < 2; ++thr)
		{
			auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(thr));
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
		if (game::Input::GetMouseDown(game::Input::MouseType::Left))
		{
			//std::cout << "click\n";
		}
	}

	void Viewport::ChangeViewportSize()
	{
		if (viewportDescSet[GAME_THREAD])
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[GAME_THREAD]);
		if (viewportWidthLast != 0.f && viewportHeightLast != 0.f)
			renderTex->SetSize(viewportWidthLast, viewportHeightLast);
		auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(GAME_THREAD));
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
			ChangeViewportSize();
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

		renderTex->Destroy();
	}

	void Viewport::SetDirty()
	{
		if (bDirty)
			return;

		bDirty = true;
		world.renderer.GetThreadSyncManager().PushSyncable(*this);
	}
	void Viewport::Sync()
	{
		std::swap(viewportDescSet[RENDER_THREAD], viewportDescSet[GAME_THREAD]);
		bDirty = false;
	}
}//namespace