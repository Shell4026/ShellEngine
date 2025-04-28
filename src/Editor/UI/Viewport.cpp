#include "UI/Viewport.h"
#include "EditorWorld.h"

#include "Core/Logger.h"
#include "Core/ThreadSyncManager.h"

#include "Game/ImGUImpl.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/Component/EditorCamera.h"
#include "Game/Component/PickingRenderer.h"

#include "Render/RenderTexture.h"
#include "Render/VulkanImpl/VulkanTextureBuffer.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"

namespace sh::editor
{
	Viewport::Viewport(EditorWorld& world) :
		world(world),

		renderTex(),
		x(0.f), y(0.f),
		viewportWidthLast(100.f), viewportHeightLast(100.f),
		bDirty(false), bMouseLeftDown(false), bMouseRightDown(false), bFocus(false)
	{
		renderTex = world.GetGameObject("EditorCamera")->GetComponent<game::EditorCamera>()->GetRenderTexture();
		outlineTex = static_cast<render::RenderTexture*>(world.textures.GetResource("OutlineTexture"));

		auto vkTexBuffer = static_cast<render::vk::VulkanTextureBuffer*>(renderTex->GetTextureBuffer());
		auto imgBuffer = vkTexBuffer->GetImageBuffer();

		viewportTexture = ImGui_ImplVulkan_AddTexture(imgBuffer->GetSampler(), imgBuffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		pickingListener.SetCallback([&world](game::PickingCamera::PixelData pixel)
			{
				//SH_INFO_FORMAT("Pick R:{}, G:{}, B:{}, A:{}", pixel.r, pixel.g, pixel.b, pixel.a);
				bool bMultiSelect = game::Input::GetKeyDown(game::Input::KeyCode::Shift);
				if (!bMultiSelect)
					world.ClearSelectedObjects();

				uint32_t id = pixel;
				if (id == 0)
				{
					if (!bMultiSelect)
						world.ClearSelectedObjects();
				}
				else if (auto pickingRenderer = game::PickingIdManager::Get(id); pickingRenderer != nullptr)
				{
					world.AddSelectedObject(&pickingRenderer->gameObject);
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

	SH_EDITOR_API void Viewport::BlockLeftClick(bool bBlock)
	{
		bBlockLeftClick = bBlock;
	}

	SH_EDITOR_API void Viewport::BlockRightClick(bool bBlock)
	{
		bBlockRightClick = bBlock;
	}

	SH_EDITOR_API void Viewport::Update()
	{
		editorCamera->SetFocus(false);
		if (!bFocus)
			return;
		editorCamera->SetFocus(true);

		if (game::Input::GetKeyDown(game::Input::KeyCode::LAlt))
			return;

		if (!bMouseLeftDown)
		{
			bMouseLeftDown = game::Input::GetMouseDown(game::Input::MouseType::Left);
			if (bMouseLeftDown && !bBlockLeftClick)
				LeftClick();
		}
		else
			bMouseLeftDown = game::Input::GetMouseDown(game::Input::MouseType::Left);
		if (!bMouseRightDown)
		{
			bMouseRightDown = game::Input::GetMouseDown(game::Input::MouseType::Right);
			if (bMouseRightDown && !bBlockRightClick)
				RightClick();
		}
		else
			bMouseRightDown = game::Input::GetMouseDown(game::Input::MouseType::Right);
	}

	void Viewport::ChangeViewportSize()
	{
		if (viewportWidthLast != 0.f && viewportHeightLast != 0.f)
			renderTex->SetSize(viewportWidthLast, viewportHeightLast); // renderTex dirty등록
		if (pickingCamera)
			pickingCamera->SetTextureSize({ viewportWidthLast, viewportHeightLast });

		SyncDirty();
	}

	void Viewport::RenderOverlay()
	{
		ImGuiChildFlags childFlags =
			ImGuiChildFlags_::ImGuiChildFlags_AutoResizeX |
			ImGuiChildFlags_::ImGuiChildFlags_AutoResizeY |
			ImGuiChildFlags_::ImGuiChildFlags_AlwaysAutoResize |
			ImGuiChildFlags_::ImGuiChildFlags_Border;
		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs;
		ImGui::SetNextWindowPos({ x, y });
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		if (ImGui::BeginChild("Viewport Overlay", { 0, 0 }, childFlags, windowFlags))
		{
			ImGui::Text(fmt::format("Render Call: {}", world.renderer.GetDrawCall(core::ThreadType::Render)).c_str());
		}
		ImGui::EndChild();
	}

	void Viewport::RenderPopup()
	{
		if (bBlockRightClick)
			return;
		if (ImGui::BeginPopupContextItem("ViewportRightClick", ImGuiPopupFlags_::ImGuiPopupFlags_MouseButtonRight))
		{
			auto& selectedObjs = world.GetSelectedObjects();
			if (selectedObjs.size() > 0)
			{
				if (ImGui::Selectable("Delete"))
				{
					for (auto obj : selectedObjs)
					{
						if (obj->GetType() == game::GameObject::GetStaticType())
						{
							if (core::IsValid(obj))
								world.DestroyGameObject(*static_cast<game::GameObject*>(obj));
						}
					}
				}
			}
			ImGui::EndPopup();
		}
	}

	void Viewport::LeftClick()
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

	void Viewport::RightClick()
	{
	}

	void Viewport::Render()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin(name);
		imguiDrawList = ImGui::GetWindowDrawList();
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
			
			if (viewportWidthLast == 0 || viewportHeightLast == 0)
			{
				ImGui::End();
				ImGui::PopStyleVar();
				return;
			}
			ChangeViewportSize();
		}
		ImGui::Image(viewportTexture, { width, height });
		ImGui::PopStyleVar();

		RenderPopup();
		RenderOverlay();

		ImGui::End();
	}

	SH_EDITOR_API auto Viewport::GetRenderTexture() -> render::RenderTexture&
	{
		return *renderTex;
	}

	SH_EDITOR_API void Viewport::Clean()
	{
		if (viewportTexture)
		{
			ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(viewportTexture));
			viewportTexture = nullptr;
		}
	}

	SH_EDITOR_API void Viewport::SyncDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this, 1);

		bDirty = true;
	}
	SH_EDITOR_API auto Viewport::Play() -> bool
	{
		game::Camera* mainCam = world.GetMainCamera();
		if (!core::IsValid(mainCam))
		{
			for (auto cam : world.GetCameras())
			{
				if (cam->GetType() == game::EditorCamera::GetStaticType() || cam->GetType() == game::PickingCamera::GetStaticType())
					continue;
				world.SetMainCamera(cam);
				mainCam = world.GetMainCamera();
				break;
			}
			if (world.GetMainCamera() == nullptr)
			{
				SH_ERROR("There are no cameras.");
				return false;
			}
		}
		if (mainCam->GetRenderTexture() != nullptr)
		{
			SH_ERROR("There are no cameras.");
			return false;
		}
		renderTex = static_cast<render::RenderTexture*>(world.textures.GetResource("GameView"));
		mainCam->SetRenderTexture(renderTex);
		ChangeViewportSize();

		world.ClearSelectedObjects();

		world.AddBeforeSyncTask([&]{ world.Playing(); });

		editorCamera->SetActive(false);
		BlockLeftClick(true);
		BlockRightClick(true);

		SyncDirty();

		return true;
	}
	SH_EDITOR_API void Viewport::Stop()
	{
		game::Camera* cam = world.GetMainCamera();
		cam->SetRenderTexture(nullptr);

		renderTex = world.GetGameObject("EditorCamera")->GetComponent<game::EditorCamera>()->GetRenderTexture();
		ChangeViewportSize();

		editorCamera->SetActive(true);
		BlockLeftClick(false);
		BlockRightClick(false);

		world.AddBeforeSyncTask([&] { world.Stop(); });

		SyncDirty();
	}
	SH_EDITOR_API void Viewport::Sync()
	{
		ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(viewportTexture));
		VkDescriptorSet viewportDescSetLast = static_cast<VkDescriptorSet>(viewportTexture);

		auto vkTexBuffer = static_cast<render::vk::VulkanTextureBuffer*>(renderTex->GetTextureBuffer());
		auto imgBuffer = vkTexBuffer->GetImageBuffer();
		viewportTexture = ImGui_ImplVulkan_AddTexture(imgBuffer->GetSampler(), imgBuffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (viewportDescSetLast != nullptr)
		{
			for (auto& cmdBuffer : imguiDrawList->CmdBuffer)
			{
				if (cmdBuffer.TextureId == (ImTextureID)viewportDescSetLast)
					cmdBuffer.TextureId = viewportTexture;
			}
			viewportDescSetLast = nullptr;
		}
		bDirty = false;
	}
}//namespace