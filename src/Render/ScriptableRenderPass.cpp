#include "ScriptableRenderPass.h"
#include "IRenderContext.h"
#include "Drawable.h"
#include "RenderTexture.h"
#include "MaterialData.h"

#include "Core/Logger.h"
#include "Core/Util.h"

namespace sh::render
{
	ScriptableRenderPass::ScriptableRenderPass(const core::Name& passName, RenderQueue renderQueue) :
		passName(passName),
		renderQueue(renderQueue)
	{
	}
	SH_RENDER_API auto ScriptableRenderPass::CreateRenderBatch(const core::Name& passName, const std::vector<Drawable*>& drawables) -> std::vector<RenderBatch>
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

		std::map<const Material*, bool> materialFilter;
		for (const Drawable* drawable : drawables)
		{
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid())
				continue;

			const Material* const mat = drawable->GetMaterial();
			if (auto it = materialFilter.find(mat); it == materialFilter.end())
			{
				if (mat->GetShader()->GetShaderPasses(passName) == nullptr)
				{
					materialFilter.insert({ mat, false });
					continue;
				}
				materialFilter.insert({ mat, true });
			}
			else
			{
				if (!it->second)
					continue;
			}

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
	SH_RENDER_API void ScriptableRenderPass::Configure(const RenderData& renderData)
	{
		if (renderData.GetDrawablesPtr() != nullptr)
		{
			renderBatches = CreateRenderBatch(passName, *renderData.GetDrawablesPtr());
		}
		SetImageUsages(renderData);
	}
	SH_RENDER_API void ScriptableRenderPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
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
		//ctx.GetRenderImpl().RecordCommand(cmd, passName, renderTarget, drawList, bStoreImage);
	}
	SH_RENDER_API void ScriptableRenderPass::SetImageUsages(const RenderData& renderData)
	{
		renderTextures.clear();

		SetRenderTargetImageUsages(renderData);

		if (renderData.GetDrawablesPtr() != nullptr)
			SetImageUsages(*renderData.GetDrawablesPtr());
	}
	SH_RENDER_API void ScriptableRenderPass::SetRenderTargetImageUsages(const RenderData& renderData)
	{
		for (const RenderTexture* target : renderData.GetRenderTargets())
		{
			if (target == nullptr)
			{
				renderTextures[nullptr] = ResourceUsage::ColorAttachment;
				continue;
			}

			renderTextures[target] = target->IsDepthTexture() ? ResourceUsage::DepthStencilAttachment : ResourceUsage::ColorAttachment;
		}
	}
	SH_RENDER_API void ScriptableRenderPass::SetImageUsages(const std::vector<Drawable*>& drawables)
	{
		for (const Drawable* drawable : drawables)
		{
			if (drawable == nullptr || !drawable->CheckAssetValid())
				continue;

			for (const MaterialData::CachedRT& cachedRT : IRenderThrMethod<MaterialData>::GetCachedRTs(drawable->GetMaterialData()))
			{
				if (cachedRT.rt.IsValid() && cachedRT.pass->GetLightingPassName() == passName)
					renderTextures[cachedRT.rt.Get()] = cachedRT.rt->IsDepthTexture() ? ResourceUsage::DepthStencilSampledRead : ResourceUsage::SampledRead;
			}
			const Material* const mat = drawable->GetMaterial();
			SetImageUsages(*mat);
		}
	}
	SH_RENDER_API void ScriptableRenderPass::SetImageUsages(const Material& mat)
	{
		for (const MaterialData::CachedRT& cachedRT : IRenderThrMethod<MaterialData>::GetCachedRTs(mat.GetMaterialData()))
		{
			if (cachedRT.rt.IsValid() && cachedRT.pass->GetLightingPassName() == passName)
				renderTextures[cachedRT.rt.Get()] = cachedRT.rt->IsDepthTexture() ? ResourceUsage::DepthStencilSampledRead : ResourceUsage::SampledRead;
		}
	}
	SH_RENDER_API void ScriptableRenderPass::SetViewportScissor(CommandBuffer& cmd, const IRenderContext& ctx, const RenderViewer& renderViewer)
	{
		cmd.SetViewport(renderViewer.viewportRect.x, renderViewer.viewportRect.y, renderViewer.viewportRect.z, renderViewer.viewportRect.w);
		cmd.SetScissor(renderViewer.viewportScissor.x, renderViewer.viewportScissor.y, renderViewer.viewportScissor.z, renderViewer.viewportScissor.w);
	}
}//namespace
