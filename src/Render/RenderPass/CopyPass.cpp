#include "RenderPass/CopyPass.h"
#include "RenderTexture.h"
#include "CommandBuffer.h"

namespace sh::render
{
	CopyPass::CopyPass() :
		ScriptableRenderPass(core::Name{"CopyPass"}, RenderQueue::AfterRendering)
	{
	}

	SH_RENDER_API void CopyPass::Configure(const RenderTarget& renderData)
	{
		if (cpyDatas.empty())
			return;

		renderTextures.clear();
		for (const CopyData& cpyData : cpyDatas)
			renderTextures[cpyData.src] = ResourceUsage::TransferSrc;
	}
	SH_RENDER_API void CopyPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData)
	{
		if (cpyDatas.empty())
			return;

		for (const CopyData& cpyData : cpyDatas)
			cmd.Blit(*cpyData.src, cpyData.x, cpyData.y, *cpyData.dst);
		cpyDatas.clear();
	}

	SH_RENDER_API void CopyPass::EnqueCopyImagePixelToBuffer(RenderTexture& rt, int x, int y, IBuffer& buffer)
	{
		cpyDatas.push_back({ &rt, &buffer, x, y });
	}
}//namespace