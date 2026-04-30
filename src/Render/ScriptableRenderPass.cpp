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
		if (renderData.drawables != nullptr)
		{
			renderBatches = CreateRenderBatch(*renderData.drawables);
		}
		SetImageUsages(renderData);
	}
	SH_RENDER_API void ScriptableRenderPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderTarget)
	{
		cmd.SetRenderTarget(renderTarget, true, true, true, true);
		SetViewportScissor(cmd, ctx, renderTarget);

		if (renderTarget.drawables == nullptr)
			return;

		for (const RenderBatch& batch : renderBatches)
		{
			cmd.DrawMeshBatch(batch.drawables, passName);
		}

		//ctx.GetRenderImpl().RecordCommand(cmd, passName, renderTarget, drawList, bStoreImage);
	}
	SH_RENDER_API auto ScriptableRenderPass::CreateRenderBatch(const std::vector<Drawable*>& drawables) const -> std::vector<RenderBatch>
	{
		std::vector<RenderBatch> groups;
		struct GroupKey
		{
			const Material* mat;
			Mesh::Topology topology;
			bool bSkinned;

			bool operator==(const GroupKey& other) const noexcept { return mat == other.mat && topology == other.topology && bSkinned == other.bSkinned; }
		};
		struct GroupKeyHash
		{
			std::size_t operator()(const GroupKey& k) const noexcept
			{
				const std::size_t hash0 = core::Util::CombineHash(std::hash<const Material*>{}(k.mat), std::hash<int>{}(static_cast<int>(k.topology)));
				const std::size_t hash1 = core::Util::CombineHash(hash0, std::hash<bool>{}(k.bSkinned));
				return hash1;
			}
		};
		std::unordered_map<GroupKey, std::size_t, GroupKeyHash> groupIndex;
		groupIndex.reserve(drawables.size());

		for (const Drawable* drawable : drawables)
		{
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid())
				continue;

			const Material* const mat = drawable->GetMaterial();
			Mesh::Topology topology = drawable->GetTopology(core::ThreadType::Render);
			const bool bSkinned = drawable->IsSkinnedMesh();

			GroupKey key{ mat, topology, bSkinned };
			auto it = groupIndex.find(key);
			if (it == groupIndex.end())
			{
				RenderBatch group{};
				group.material = mat;
				group.topology = topology;
				group.bSkinned = bSkinned;
				group.drawables.push_back(drawable);

				groupIndex.emplace(key, groups.size());
				groups.push_back(std::move(group));
			}
			else
			{
				groups[it->second].drawables.push_back(drawable);
			}
		}
		return groups;
	}
	SH_RENDER_API void ScriptableRenderPass::SetImageUsages(const RenderTarget& renderData)
	{
		renderTextures.clear();
		if (renderData.target != nullptr && IsDepthTexture(renderData.target->GetTextureFormat()))
			renderTextures[renderData.target] = ResourceUsage::DepthStencilAttachment;
		else
			renderTextures[renderData.target] = ResourceUsage::ColorAttachment;

		if (renderData.drawables == nullptr)
			return;

		for (const Drawable* drawable : *renderData.drawables)
		{
			if (drawable == nullptr || !drawable->CheckAssetValid())
				continue;

			const Material* const mat = drawable->GetMaterial();
			for (auto& [name, rt] : mat->GetCachedRenderTextures())
			{
				auto it = renderTextures.find(rt);
				if (it != renderTextures.end())
				{
					if (it->second == ResourceUsage::ColorAttachment ||
						it->second == ResourceUsage::DepthStencilAttachment)
					{
						SH_ERROR_FORMAT("RenderTexture {} is used as Attachment", it->first->GetName().ToString());
						continue;
					}
				}
				else
				{
					renderTextures[rt] = rt->IsDepthOnly() ? ResourceUsage::DepthStencilSampledRead : ResourceUsage::SampledRead;
				}
			}
		}
	}
	SH_RENDER_API void ScriptableRenderPass::SetViewportScissor(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderTarget)
	{
		if (renderTarget.target == nullptr)
		{
			const int x = static_cast<int>(ctx.GetViewportStart().x);
			const int y = static_cast<int>(ctx.GetViewportEnd().y);
			const int w = static_cast<int>(ctx.GetViewportEnd().x - ctx.GetViewportStart().x);
			const int h = static_cast<int>(ctx.GetViewportEnd().y - ctx.GetViewportStart().y);
			cmd.SetViewport(x, y, w, -h);
			cmd.SetScissor(0, 0, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
		}
		else
		{
			const int x = 0;
			const int y = static_cast<int>(renderTarget.target->GetSize().y);
			const int w = static_cast<int>(renderTarget.target->GetSize().x);
			const int h = y;
			cmd.SetViewport(x, y, w, -h);
			cmd.SetScissor(0, 0, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
		}
	}
}//namespace