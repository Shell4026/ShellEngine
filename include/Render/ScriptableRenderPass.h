#pragma once
#include "Export.h"
#include "RenderData.h"
#include "IRenderThrMethod.h"

#include "Core/SContainer.hpp"

#include <unordered_map>
namespace sh::render
{
	class IRenderContext;
	class CommandBuffer;
	class RenderTexture;

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
		static void Configure(ScriptableRenderPass& pass, const RenderTarget& renderData);
		static void Record(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData);
		static void EmitBarrier(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const std::vector<BarrierInfo>& barriers);
	};

	class ScriptableRenderPass
	{
		friend IRenderThrMethod<ScriptableRenderPass>;
	public:
		SH_RENDER_API ScriptableRenderPass(const core::Name& passName, RenderQueue renderQueue);
		SH_RENDER_API virtual ~ScriptableRenderPass() = default;

		SH_RENDER_API auto GetRenderTextures() const -> const std::unordered_map<const RenderTexture*, ImageUsage>& { return renderTextures; }
	protected:
		SH_RENDER_API virtual void Configure(const RenderTarget& renderData);
		SH_RENDER_API virtual void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData);
		SH_RENDER_API virtual auto BuildDrawList(const RenderTarget& renderData) -> DrawList;

		SH_RENDER_API void EmitBarrier(CommandBuffer& cmd, const IRenderContext& ctx, const std::vector<BarrierInfo>& barriers);
	private:
		void CollectRenderImages(const RenderTarget& renderData, const DrawList& drawList);
	public:
		const core::Name passName;
		const RenderQueue renderQueue;
	protected:
		DrawList drawList;
		std::unordered_map<const RenderTexture*, ImageUsage> renderTextures;
	};

	inline void IRenderThrMethod<ScriptableRenderPass>::Configure(ScriptableRenderPass& pass, const RenderTarget& renderData)
	{
		pass.Configure(renderData);
	}
	inline void IRenderThrMethod<ScriptableRenderPass>::Record(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData)
	{
		pass.Record(cmd, ctx, renderData);
	}
	inline void IRenderThrMethod<ScriptableRenderPass>::EmitBarrier(ScriptableRenderPass& pass, CommandBuffer& cmd, const IRenderContext& ctx, const std::vector<BarrierInfo>& barriers)
	{
		pass.EmitBarrier(cmd, ctx, barriers);
	}
}//namespace