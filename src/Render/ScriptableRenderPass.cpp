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
		list.renderData = std::vector<DrawList::RenderGroup>{};
		if (renderData.drawables == nullptr)
			return list;

		for (auto drawable : *renderData.drawables)
		{
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid() || drawable->GetMaterial()->GetShader()->GetShaderPasses(passName) == nullptr)
				continue;

			const Material* mat = drawable->GetMaterial();
			Mesh::Topology topology = drawable->GetTopology(core::ThreadType::Render);

			DrawList::RenderGroup* renderGroup = nullptr;
			for (auto& group : std::get<0>(list.renderData))
			{
				if (group.material == mat && group.topology == topology)
				{
					renderGroup = &group;
					break;
				}
			}
			if (renderGroup == nullptr)
			{
				DrawList::RenderGroup group{};
				group.material = mat;
				group.topology = topology;
				group.drawables.push_back(drawable);

				std::get<0>(list.renderData).push_back(std::move(group));
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

	SH_RENDER_API void ScriptableRenderPass::SetStoreImg(const IRenderContext& ctx, bool bStore)
	{
		IRenderThrMethod<IRenderImpl>::SetStoreImage(ctx.GetRenderImpl(), bStore);
	}

	void ScriptableRenderPass::CollectRenderImages(const RenderTarget& renderData, const DrawList& drawList)
	{
		renderTextures.clear();
		renderTextures[renderData.target] = ImageUsage::ColorAttachment;

		const auto collectFn =
			[this](const Material* mat)
			{
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
			};

		if (drawList.renderData.index() == 0)
		{
			for (const auto& group : std::get<0>(drawList.renderData))
				collectFn(group.material);
		}
		else
		{
			for (const auto& item : std::get<1>(drawList.renderData))
				collectFn(item.material);
		}
	}
}//namespace