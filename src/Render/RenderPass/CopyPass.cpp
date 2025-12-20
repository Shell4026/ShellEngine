#include "RenderPass/CopyPass.h"
#include "RenderTexture.h"

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

		drawList = BuildDrawList(renderData);

		renderTextures.clear();
		for (auto& cpyData : cpyDatas)
			renderTextures[cpyData.src] = ImageUsage::Src;
	}
	SH_RENDER_API void CopyPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData)
	{
		if (cpyDatas.empty())
			return;

		RenderTarget data{};
		data.frameIndex = 0;
		data.camera = nullptr;
		data.target = nullptr;
		data.drawables = nullptr;

		ScriptableRenderPass::Record(cmd, ctx, data);
		cpyDatas.clear();
	}
	SH_RENDER_API auto CopyPass::BuildDrawList(const RenderTarget& renderData) -> DrawList
	{
		DrawList list{};
		list.drawCall.push_back(
			[cpyDatas = cpyDatas](CommandBuffer& cmd)
			{
				for (auto& cpyData : cpyDatas)
					cmd.Blit(*cpyData.src, cpyData.x, cpyData.y, *cpyData.dst);
			}
		);
		return list;
	}

	SH_RENDER_API void CopyPass::EnqueCopyImagePixelToBuffer(RenderTexture& rt, int x, int y, IBuffer& buffer)
	{
		cpyDatas.push_back({ &rt, &buffer, x, y });
	}
}//namespace