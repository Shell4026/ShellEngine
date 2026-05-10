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
		// 한 프레임 내 첫 호출에서만 클리어
		if (renderData.GetFrameIdx() != lastFrameIndex)
		{
			lastFrameIndex = renderData.GetFrameIdx();
			callsThisFrame = 0;
		}
		const bool bClearDepth = (callsThisFrame == 0);
		++callsThisFrame;

		cmd.SetRenderData(renderData, false, bClearDepth, false, true);

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
