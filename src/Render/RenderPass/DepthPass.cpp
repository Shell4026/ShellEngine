#include "RenderPass/DepthPass.h"
#include "CommandBuffer.h"
#include "IRenderContext.h"

namespace sh::render
{
	DepthPass::DepthPass() :
		ScriptableRenderPass(core::Name{ "DepthPass" }, RenderQueue::BeforeRendering)
	{
	}

	SH_RENDER_API void DepthPass::Configure(const RenderData& renderData)
	{
		ScriptableRenderPass::Configure(renderData);
	}
	SH_RENDER_API void DepthPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
	{
		cmd.SetRenderData(renderData, true, true, true, true);

		if (renderData.GetDrawablesPtr() == nullptr)
			return;

		std::size_t viewerIdx = 0;
		for (const RenderViewer& viewer : renderData.renderViewers)
		{
			SetViewportScissor(cmd, ctx, viewer);
			for (const RenderBatch& batch : renderBatches)
				cmd.DrawMeshBatch(batch.drawables, passName, viewerIdx);
			++viewerIdx;
		}
	}
}//namespace
