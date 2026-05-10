#include "GUIPass.h"
#include "ImGUImpl.h"

#include "Render/IRenderContext.h"
#include "Render/VulkanImpl/VulkanCommandBuffer.h"

#include "External/imgui/backends/imgui_impl_vulkan.h"

namespace sh::game
{
	GUIPass::GUIPass() :
		render::ScriptableRenderPass(core::Name{ "ImGUI" }, render::RenderQueue::UI)
	{
	}
	SH_GAME_API void GUIPass::SetImGUIContext(ImGUImpl& gui)
	{
		this->gui = &gui;
	}
	SH_GAME_API void GUIPass::Configure(const render::RenderData& renderData)
	{
		renderTextures.clear();
		SetRenderTargetImageUsages(renderData);
		if (viewportTexture != nullptr)
			renderTextures[viewportTexture] = render::ResourceUsage::SampledRead;
	}

	SH_GAME_API void GUIPass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData)
	{
		cmd.SetRenderData(renderData, false, true, true, false);
		for (const render::RenderViewer& viewer : renderData.renderViewers)
		{
			SetViewportScissor(cmd, ctx, viewer);
			ImGui_ImplVulkan_RenderDrawData(&gui->GetDrawData(), static_cast<render::vk::VulkanCommandBuffer&>(cmd).GetCommandBuffer());
		}
	}
}//namespace
