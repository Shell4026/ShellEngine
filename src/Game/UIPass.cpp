#include "UIPass.h"
#include "ImGUImpl.h"

#include "Render/IRenderContext.h"
#include "Render/VulkanImpl/VulkanCommandBuffer.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"

namespace sh::game
{
	UIPass::UIPass() :
		render::ScriptableRenderPass(core::Name{ "UI" }, render::RenderQueue::UI)
	{
	}
	SH_GAME_API void UIPass::SetImGUIContext(ImGUImpl& gui)
	{
		this->gui = &gui;
	}
	SH_GAME_API void UIPass::Configure(const render::RenderTarget& renderData)
	{
		ScriptableRenderPass::Configure(renderData);
		if (viewportTexture != nullptr)
			renderTextures[viewportTexture] = render::ImageUsage::SampledRead;
	}
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
}//namespace