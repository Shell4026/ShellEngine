#include "GUITexture.h"

#include "Core/ThreadSyncManager.h"
#include "Core/Logger.h"

#include "Render/IRenderContext.h"
#include "Render/Texture.h"
#include "Render/VulkanImpl/VulkanImageBuffer.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"

#include <cassert>
namespace sh::game
{
	SH_GAME_API GUITexture::GUITexture() :
		tex(nullptr)
	{
		InitListeners();
	}
	GUITexture::GUITexture(GUITexture&& other) noexcept :
		context(other.context),
		tex(other.tex),
		originalTex(std::move(other.originalTex))
	{
		InitListeners();
		other.tex = nullptr;
		if (core::IsValid(originalTex))
		{
			originalTex->onDestroy.UnRegister(other.onDestroyListener);
			originalTex->onBufferUpdate.UnRegister(other.onBufferUpdateListener);

			originalTex->onDestroy.Register(onDestroyListener);
			originalTex->onBufferUpdate.Register(onBufferUpdateListener);
		}
	}

	GUITexture::~GUITexture()
	{
		SH_INFO("~GUITexture()");
	}

	SH_GAME_API void GUITexture::OnDestroy()
	{
		if (core::IsValid(originalTex))
		{
			originalTex->onDestroy.UnRegister(onDestroyListener);
			originalTex->onBufferUpdate.UnRegister(onBufferUpdateListener);
		}
		UpdateDrawList();

		Super::OnDestroy();
	}
	SH_GAME_API void GUITexture::Destroy()
	{
		Super::Destroy();
	}

	SH_GAME_API void GUITexture::Create(const render::Texture& texture)
	{
		Clean();
		if (tex != nullptr)
		{
			bRecreate = true;
			SyncDirty();
		}

		this->context = texture.GetContext();
		originalTex = &texture;

		texture.onDestroy.Register(onDestroyListener);
		texture.onBufferUpdate.Register(onBufferUpdateListener);
		assert(context->GetRenderAPIType() == render::RenderAPI::Vulkan);
		if (context->GetRenderAPIType() == render::RenderAPI::Vulkan)
		{
			auto buffer = static_cast<render::vk::VulkanImageBuffer*>(texture.GetTextureBuffer());
			tex = ImGui_ImplVulkan_AddTexture(buffer->GetSampler(), buffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	SH_GAME_API void GUITexture::Clean()
	{
		if (context == nullptr)
			return;

		if (core::IsValid(originalTex))
		{
			originalTex->onDestroy.UnRegister(onDestroyListener);
			originalTex->onBufferUpdate.UnRegister(onBufferUpdateListener);
		}
		originalTex = nullptr;

		SyncDirty();
	}

	SH_GAME_API void GUITexture::Draw(ImVec2 size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tintCol, const ImVec4& borderCol)
	{
		if (tex == nullptr)
			return;
		ImGui::Image(tex, size, uv0, uv1, tintCol, borderCol);
		drawList = ImGui::GetWindowDrawList();
	}

	SH_GAME_API void GUITexture::DrawButton(const char* strId, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bgCol, const ImVec4& tintCol)
	{
		if (tex == nullptr)
			return;
		ImGui::ImageButton(strId, tex, size, uv0, uv1, bgCol, tintCol);
		drawList = ImGui::GetWindowDrawList();
	}

	SH_GAME_API void GUITexture::Sync()
	{
		assert(context->GetRenderAPIType() == render::RenderAPI::Vulkan);

		if (drawList != nullptr)
			UpdateDrawList();

		drawList = nullptr;
		bDirty = false;
		bRecreate = false;
	}

	SH_GAME_API void GUITexture::SyncDirty()
	{
		if (bDirty)
			return;
		core::ThreadSyncManager::PushSyncable(*this, -1);
		bDirty = true;
	}

	void GUITexture::InitListeners()
	{
		onDestroyListener.SetCallback(
			[&](const core::SObject* obj)
			{
				Clean();
			}
		);
		onBufferUpdateListener.SetCallback(
			[&](const render::Texture* texture)
			{
				bRecreate = true;
				SyncDirty();
			}
		);
	}

	void GUITexture::UpdateDrawList()
	{
		ImTextureID oldTexHandle = tex;
		if (tex != nullptr)
		{
			if (context->GetRenderAPIType() == render::RenderAPI::Vulkan)
				ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(tex));

			oldTexHandle = tex;
		}
		tex = nullptr;

		if (bRecreate)
		{
			if (core::IsValid(originalTex))
			{
				if (context->GetRenderAPIType() == render::RenderAPI::Vulkan)
				{
					auto buffer = static_cast<render::vk::VulkanImageBuffer*>(originalTex->GetTextureBuffer());
					tex = ImGui_ImplVulkan_AddTexture(buffer->GetSampler(), buffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				if (oldTexHandle != nullptr)
				{
					for (auto& cmdBuffer : drawList->CmdBuffer)
					{
						if (cmdBuffer.TextureId == oldTexHandle)
							cmdBuffer.TextureId = tex;
					}
				}
			}
		}
		else
		{
			if (oldTexHandle != nullptr && drawList != nullptr)
			{
				int idx = -1;
				for (int i = 0; i < drawList->CmdBuffer.size(); ++i)
				{
					if (drawList->CmdBuffer[i].TextureId == oldTexHandle)
						idx = i;
				}
				ImVector<ImDrawCmd> newCmd;
				if (idx != -1)
				{
					for (int i = 0; i < drawList->CmdBuffer.size(); ++i)
					{
						if (i == idx)
							continue;
						newCmd.push_back(drawList->CmdBuffer[i]);
					}
					drawList->CmdBuffer = newCmd;
				}
			}
		}
	}

	SH_GAME_API auto GUITexture::IsValid() const -> bool
	{
		return tex != nullptr;
	}
}//namespace