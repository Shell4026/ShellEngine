#include "RenderPass/ShadowMapPass.h"
#include "CommandBuffer.h"
#include "IRenderContext.h"

namespace sh::render
{
	ShadowMapPass::ShadowMapPass() :
		ScriptableRenderPass(core::Name{ "ShadowMapPass" }, RenderQueue::BeforeRendering)
	{
	}

	SH_RENDER_API void ShadowMapPass::Configure(const RenderData& renderData)
	{
		ScriptableRenderPass::Configure(renderData);
	}
	SH_RENDER_API void ShadowMapPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
	{
		if (renderData.target == nullptr)
			return;

		// 한 프레임 내 첫 그림자 호출에서만 아틀라스를 클리어하고, 이후엔 LOAD하여 다른 광원의 슬롯 결과를 보존
		if (renderData.frameIndex != lastFrameIndex)
		{
			lastFrameIndex = renderData.frameIndex;
			callsThisFrame = 0;
		}
		const bool bClearDepth = (callsThisFrame == 0);
		++callsThisFrame;

		cmd.SetRenderData(renderData, false, bClearDepth, false, true);

		if (renderData.drawables == nullptr)
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
