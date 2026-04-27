#include "ScriptableRenderPass.h"
#include "IRenderContext.h"
#include "Drawable.h"
#include "RenderTexture.h"

#include "Core/Logger.h"
#include "Core/Util.h"

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
		ctx.GetRenderImpl().RecordCommand(cmd, passName, renderData, drawList, bStoreImage);
	}
	SH_RENDER_API auto ScriptableRenderPass::BuildDrawList(const RenderTarget& renderData) -> DrawList
	{
		DrawList list{};
		list.renderData = std::vector<DrawList::RenderGroup>{};
		if (renderData.drawables == nullptr)
			return list;

		std::vector<DrawList::RenderGroup>& groups = std::get<std::vector<DrawList::RenderGroup>>(list.renderData);

		struct GroupKey
		{
			const Material* mat;
			Mesh::Topology topology;
			bool operator==(const GroupKey& other) const noexcept { return mat == other.mat && topology == other.topology; }
		};
		struct GroupKeyHash
		{
			std::size_t operator()(const GroupKey& k) const noexcept
			{
				return core::Util::CombineHash(std::hash<const Material*>{}(k.mat), std::hash<int>{}(static_cast<int>(k.topology)));
			}
		};
		std::unordered_map<GroupKey, std::size_t, GroupKeyHash> groupIndex;
		groupIndex.reserve(renderData.drawables->size());

		for (Drawable* drawable : *renderData.drawables)
		{
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid() || drawable->GetMaterial()->GetShader()->GetShaderPasses(passName) == nullptr)
				continue;

			const Material* const mat = drawable->GetMaterial();
			Mesh::Topology topology = drawable->GetTopology(core::ThreadType::Render);

			GroupKey key{ mat, topology };
			auto it = groupIndex.find(key);
			if (it == groupIndex.end())
			{
				DrawList::RenderGroup group{};
				group.material = mat;
				group.topology = topology;
				group.drawables.push_back(drawable);

				groupIndex.emplace(key, groups.size());
				groups.push_back(std::move(group));
			}
			else
			{
				groups[it->second].drawables.push_back(drawable);
			}
			++list.drawableCount;
		}
		return list;
	}
	SH_RENDER_API void ScriptableRenderPass::EmitBarrier(CommandBuffer& cmd, const IRenderContext& ctx, const std::vector<BarrierInfo>& barriers) const
	{
		ctx.GetRenderImpl().EmitBarrier(cmd, barriers);
	}
	SH_RENDER_API auto ScriptableRenderPass::GetRenderCallCount() const -> uint32_t
	{
		return static_cast<uint32_t>(drawList.drawCall.size()) + drawList.drawableCount;
	}

	void ScriptableRenderPass::CollectRenderImages(const RenderTarget& renderData, const DrawList& drawList)
	{
		renderTextures.clear();
		renderTextures[renderData.target] = ResourceUsage::ColorAttachment;

		const auto collectFn =
			[this](const Material* mat)
			{
				for (auto& [name, rt] : mat->GetCachedRenderTextures())
				{
					auto it = renderTextures.find(rt);
					if (it != renderTextures.end())
					{
						if (it->second == ResourceUsage::ColorAttachment)
						{
							SH_ERROR_FORMAT("RenderTexture {} is used as ColorAttachment", it->first->GetName().ToString());
							continue;
						}
					}
					else
						renderTextures[rt] = ResourceUsage::SampledRead;
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