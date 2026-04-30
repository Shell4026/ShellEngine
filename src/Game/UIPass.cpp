#include "UIPass.h"
#include "ImGUImpl.h"

#include "Render/IRenderContext.h"
#include "Render/VulkanImpl/VulkanCommandBuffer.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"

namespace sh::game
{
	UIPass::UIPass() :
		render::ScriptableRenderPass(core::Name{ "ImGUI" }, render::RenderQueue::UI)
	{
	}
	SH_GAME_API void UIPass::SetImGUIContext(ImGUImpl& gui)
	{
		this->gui = &gui;
	}
	SH_GAME_API void UIPass::Configure(const render::RenderTarget& renderData)
	{
		renderTextures[renderData.target] = render::ResourceUsage::ColorAttachment;
		if (viewportTexture != nullptr)
			renderTextures[viewportTexture] = render::ResourceUsage::SampledRead;
	}

	SH_GAME_API void UIPass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderTarget& renderData)
	{
		ScriptableRenderPass::Record(cmd, ctx, renderData);
		ImGui_ImplVulkan_RenderDrawData(&gui->GetDrawData(), static_cast<render::vk::VulkanCommandBuffer&>(cmd).GetCommandBuffer());
	}
}//namespace