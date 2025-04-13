#include "PCH.h"
#include "GUITexture.h"

#include "Render/IRenderContext.h"
#include "Render/Texture.h"
#include "Render/VulkanImpl/VulkanTextureBuffer.h"

namespace sh::game
{
	SH_GAME_API GUITexture::GUITexture() :
		tex(nullptr)
	{
		
	}
	GUITexture::~GUITexture()
	{
		Clean();
	}

	SH_GAME_API void GUITexture::Create(const render::IRenderContext& context, const render::Texture& texture)
	{
		this->context = &context;
		if (context.GetRenderAPIType() == render::RenderAPI::Vulkan)
		{
			auto buffer = static_cast<render::vk::VulkanTextureBuffer*>(texture.GetTextureBuffer())->GetImageBuffer();
			tex = ImGui_ImplVulkan_AddTexture(buffer->GetSampler(), buffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	SH_GAME_API void GUITexture::Clean()
	{
		if (tex == nullptr || context == nullptr)
			return;
		if (context->GetRenderAPIType() == render::RenderAPI::Vulkan)
			ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(tex));

		tex = nullptr;
	}

	SH_GAME_API GUITexture::operator ImTextureID() const
	{
		return tex;
	}

	SH_GAME_API bool GUITexture::IsValid() const
	{
		return tex != nullptr;
	}
}//namespace