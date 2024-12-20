﻿#include "Viewport.h"
#include "EditorWorld.h"

#include "Core/Logger.h"

#include "Game/ImGUImpl.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingRenderer.h"

#include "Render/RenderTexture.h"
#include "Render/VulkanImpl/VulkanTextureBuffer.h"

namespace sh::editor
{
	Viewport::Viewport(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui),
		world(world),

		viewportDescSet(), renderTex(),
		x(0.f), y(0.f),
		viewportWidthLast(100.f), viewportHeightLast(100.f),
		bDirty(false), bMouseDown(false), bFocus(false)
	{
		renderTex = core::SObject::Create<render::RenderTexture>();
		renderTex->Build(world.renderer);
		core::GarbageCollection::GetInstance()->SetRootSet(renderTex);

		for (int thr = 0; thr < 2; ++thr)
		{
			auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(static_cast<core::ThreadType>(thr)));
			auto imgBuffer = vkTexBuffer->GetImageBuffer();

			viewportDescSet[thr] = nullptr;
		}

		pickingListener.SetCallback([&world](game::PickingCamera::PixelData pixel)
			{
				SH_INFO_FORMAT("Pick R:{}, G:{}, B:{}, A:{}", pixel.r, pixel.g, pixel.b, pixel.a);
				uint32_t id = pixel;
				if (id == 0)
				{
					world.SetSelectedObject(nullptr);
				}
				else if (auto pickingRenderer = game::PickingIdManager::Get(id); pickingRenderer != nullptr)
				{
					world.SetSelectedObject(&pickingRenderer->gameObject);
				}
			}
		);

		if (pickingCamera == nullptr)
			pickingCamera = world.GetGameObject("PickingCamera")->GetComponent<game::PickingCamera>();
		if (editorCamera == nullptr)
			editorCamera = world.GetGameObject("EditorCamera")->GetComponent<game::EditorCamera>();
	}
	Viewport::~Viewport()
	{
		Clean();
	}

	void Viewport::Update()
	{
		editorCamera->SetFocus(false);
		if (!bFocus)
			return;
		editorCamera->SetFocus(true);

		if (game::Input::GetKeyDown(game::Input::KeyCode::LAlt))
			return;

		if (!bMouseDown)
		{
			bMouseDown = game::Input::GetMouseDown(game::Input::MouseType::Left);
			if (bMouseDown)
			{
				mousePos.x = ImGui::GetIO().MousePos.x - x;
				mousePos.y = ImGui::GetIO().MousePos.y - y;
				if (mousePos.x < 0 || mousePos.y < 0)
					return;
				if (mousePos.x > viewportWidthLast || mousePos.y > viewportHeightLast)
					return;

				pickingCamera->SetPickingPos({ mousePos.x, mousePos.y });
				pickingCamera->pickingCallback.Register(pickingListener);
			}
		}
		else
			bMouseDown = game::Input::GetMouseDown(game::Input::MouseType::Left);
	}

	void Viewport::ChangeViewportSize()
	{
		//SH_INFO_FORMAT("Sampler {}, {}", 
		//	(void*)static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(core::ThreadType::Game))->GetImageBuffer()->GetSampler(), 
		//	(void*)static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(core::ThreadType::Render))->GetImageBuffer()->GetSampler());
		//SH_INFO_FORMAT("DescriptorSets {}, {}", (void*)viewportDescSet[core::ThreadType::Game], (void*)viewportDescSet[core::ThreadType::Render]);
		if (viewportDescSet[core::ThreadType::Game])
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[core::ThreadType::Game]);
		if (viewportWidthLast != 0.f && viewportHeightLast != 0.f)
			renderTex->SetSize(viewportWidthLast, viewportHeightLast);
		auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(renderTex->GetBuffer(core::ThreadType::Game));
		auto imgBuffer = vkTexBuffer->GetImageBuffer();
		viewportDescSet[core::ThreadType::Game] = ImGui_ImplVulkan_AddTexture(imgBuffer->GetSampler(), imgBuffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (pickingCamera)
			pickingCamera->SetTextureSize({ viewportWidthLast, viewportHeightLast });

		SetDirty();
	}

	void Viewport::Render()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin(name);
		bFocus = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();
		float width = ImGui::GetContentRegionAvail().x;
		float height = ImGui::GetContentRegionAvail().y;

		ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
		x = ImGui::GetWindowPos().x + contentRegionMin.x;
		y = ImGui::GetWindowPos().y + contentRegionMin.y;
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
			// 다음 동기화 타이밍 후 viewportDescSet[core::ThreadType::Render] = viewportDescSet[core::ThreadType::Game]가 되기 때문에
			// 드로우 콜에 집어 넣을 때는 미리 core::ThreadType::Game의 이미지를 넣는다.
			if (viewportDescSet[core::ThreadType::Game])
				ImGui::Image((ImTextureID)viewportDescSet[core::ThreadType::Game], { width, height });
		}
		else
		{
			if (viewportDescSet[core::ThreadType::Render])
				ImGui::Image((ImTextureID)viewportDescSet[core::ThreadType::Render], { width, height });
		}
		ImGui::End();
		ImGui::PopStyleVar();

		imgui.SetDirty();
	}

	auto Viewport::GetRenderTexture() -> render::RenderTexture&
	{
		return *renderTex;
	}

	void Viewport::Clean()
	{
		if (viewportDescSet[core::ThreadType::Game])
		{
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[core::ThreadType::Game]);
			viewportDescSet[core::ThreadType::Game] = nullptr;
		}
		if (viewportDescSet[core::ThreadType::Render])
		{
			ImGui_ImplVulkan_RemoveTexture(viewportDescSet[core::ThreadType::Render]);
			viewportDescSet[core::ThreadType::Render] = nullptr;
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
		std::swap(viewportDescSet[core::ThreadType::Render], viewportDescSet[core::ThreadType::Game]);
		bDirty = false;
	}
}//namespace