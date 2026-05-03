#pragma once
#include "Export.h"
#include "RenderData.h"
#include "IRenderThrMethod.h"
#include "Mesh.h"

#include "Core/SContainer.hpp"

#include <unordered_map>
#include <vector>
namespace sh::render
{
	class IRenderContext;
	class CommandBuffer;
	class RenderTexture;
	class Material;
	class Drawable;

	enum class RenderQueue
	{
		BeforeRendering,

		Picking,

		BeforeOpaque,
		Opaque,
		AfterOpaque,

		BeforeTransparent,
		Transparent,
		AfterTransparent,

		BeforeUI,
		UI,
		AfterUI,

		AfterRendering
	};

	template<>
	struct IRenderThrMethod<class ScriptableRenderPass>
	{
		static void Configure(ScriptableRenderPass& pass, const RenderData& renderData);
		static void Record(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderTarget);
		/// @brief Configure 이후에 호출해야 정확한 렌더콜 갯수를 알 수 있음
		/// @param pass 패스
		/// @return 렌더콜 횟수
		static auto GetRenderCallCount(ScriptableRenderPass& pass) -> uint32_t;
	};

	class ScriptableRenderPass
	{
		friend IRenderThrMethod<ScriptableRenderPass>;
	public:
		SH_RENDER_API ScriptableRenderPass(const core::Name& passName, RenderQueue renderQueue);
		SH_RENDER_API virtual ~ScriptableRenderPass() = default;

		SH_RENDER_API auto GetRenderTextures() const -> const std::unordered_map<const RenderTexture*, ResourceUsage>& { return renderTextures; }
	protected:
		struct RenderBatch
		{
			const Material* material;
			Mesh::Topology topology;
			bool bSkinned = false;
			std::vector<const Drawable*> drawables;
		};
		SH_RENDER_API virtual void Configure(const RenderData& renderData);
		SH_RENDER_API virtual void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData);

		SH_RENDER_API auto CreateRenderBatch(const std::vector<Drawable*>& drawables) const -> std::vector<RenderBatch>;
		SH_RENDER_API void SetImageUsages(const RenderData& renderData);
		SH_RENDER_API void SetImageUsages(const std::vector<Drawable*>& drawables);
		SH_RENDER_API void SetViewportScissor(CommandBuffer& cmd, const IRenderContext& ctx, const RenderViewer& renderViewer);

		/// @brief Configure 이후에 호출해야 정확한 렌더콜 갯수를 알 수 있음
		SH_RENDER_API auto GetRenderCallCount() const -> uint32_t { return renderCallCount; }
	public:
		const core::Name passName;
		const RenderQueue renderQueue;
	protected:
		std::vector<RenderBatch> renderBatches;
		std::unordered_map<const RenderTexture*, ResourceUsage> renderTextures;
	private:
		uint32_t renderCallCount = 0;
	};

	inline void IRenderThrMethod<ScriptableRenderPass>::Configure(ScriptableRenderPass& pass, const RenderData& renderData)
	{
		pass.Configure(renderData);
	}
	inline void IRenderThrMethod<ScriptableRenderPass>::Record(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
	{
		pass.Record(cmd, ctx, renderData);
	}
	inline auto IRenderThrMethod<class ScriptableRenderPass>::GetRenderCallCount(ScriptableRenderPass& pass) -> uint32_t
	{
		return pass.GetRenderCallCount();
	}
}//namespace