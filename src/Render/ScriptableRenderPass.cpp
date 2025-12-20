#include "ScriptableRenderPass.h"
#include "IRenderContext.h"
#include "Drawable.h"
#include "RenderTexture.h"
namespace sh::render
{
	ScriptableRenderPass::ScriptableRenderPass(const core::Name& passName, RenderQueue renderQueue) :
		passName(passName),
		renderQueue(renderQueue)
	{
	}
	SH_RENDER_API void ScriptableRenderPass::Configure(const RenderTarget& renderData)
	{
		drawList = BuildDrawList(renderData);
		CollectRenderImages(renderData, drawList);
	}
	SH_RENDER_API void ScriptableRenderPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData)
	{
		ctx.GetRenderImpl().RecordCommand(cmd, passName, renderData, drawList);
	}
	SH_RENDER_API auto ScriptableRenderPass::BuildDrawList(const RenderTarget& renderData) -> DrawList
	{
		DrawList list{};
		for (auto drawable : *renderData.drawables)
		{
			if (!drawable->CheckAssetValid())
				continue;
			if (drawable->GetMaterial()->GetShader()->GetShaderPasses(passName) == nullptr)
				continue;

			const Material* mat = drawable->GetMaterial();
			Mesh::Topology topology = drawable->GetTopology(core::ThreadType::Render);

			DrawList::Group* renderGroup = nullptr;
			for (auto& group : list.groups)
			{
				if (group.material == mat && group.topology == topology)
				{
					renderGroup = &group;
					break;
				}
			}
			if (renderGroup == nullptr)
			{
				DrawList::Group group{};
				group.material = mat;
				group.topology = topology;
				group.drawables.push_back(drawable);

				list.groups.push_back(std::move(group));
			}
			else
			{
				renderGroup->drawables.push_back(drawable);
			}
		}
		return list;
	}
	SH_RENDER_API void ScriptableRenderPass::EmitBarrier(CommandBuffer& cmd, const IRenderContext& ctx, const std::vector<BarrierInfo>& barriers)
	{
		ctx.GetRenderImpl().EmitBarrier(cmd, barriers);
	}

	void ScriptableRenderPass::CollectRenderImages(const RenderTarget& renderData, const DrawList& drawList)
	{
		renderTextures.clear();
		renderTextures[renderData.target] = ImageUsage::ColorAttachment;
		for (const auto& group : drawList.groups)
		{
			const Material* mat = group.material;
			for (auto& [name, rt] : mat->GetCachedRenderTextures())
			{
				auto it = renderTextures.find(rt);
				if (it != renderTextures.end())
				{
					if (it->second == ImageUsage::ColorAttachment)
					{
						SH_ERROR_FORMAT("RenderTexture {} is used as ColorAttachment", it->first->GetName().ToString());
						continue;
					}
				}
				else
					renderTextures[rt] = ImageUsage::SampledRead;
			}
		}
	}
}//namespace