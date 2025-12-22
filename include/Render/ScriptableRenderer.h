#pragma once
#include "Export.h"
#include "IRenderContext.h"
#include "RenderData.h"
#include "ScriptableRenderPass.h"
#include "IRenderThrMethod.h"
#include "IBuffer.h"
#include "RenderPass/CopyPass.h"

#include "Core/Util.h"
#include "Core/ISyncable.h"

#include <vector>
#include <type_traits>
#include <utility>
#include <future>
namespace sh::render
{
	template<>
	struct IRenderThrMethod<class ScriptableRenderer>
	{
		static void Setup(ScriptableRenderer& renderer, const RenderTarget& data);
		static void Execute(ScriptableRenderer& renderer, const RenderTarget& data);
		static void ExecuteTransfer(ScriptableRenderer& renderer, uint32_t imgIdx);
		static void EnqueRenderPass(ScriptableRenderer& renderer, ScriptableRenderPass& pass);
		static void CallReadbacks(ScriptableRenderer& renderer);
		static void ResetSubmittedCommands(ScriptableRenderer& renderer, IRenderContext& ctx);
		static void ResetSwapChainStates(ScriptableRenderer& renderer);
	};

	class ScriptableRenderer : public core::ISyncable
	{
		friend IRenderThrMethod<class ScriptableRenderer>;
	public:
		struct SubmittedCommand
		{
			ScriptableRenderPass& pass;
			CommandBuffer& cmd;
		};
	public:
		SH_RENDER_API ScriptableRenderer(IRenderContext& ctx);
		SH_RENDER_API virtual ~ScriptableRenderer() = default;

		SH_RENDER_API auto AddRenderPass(const core::Name& passName, RenderQueue renderQueue) -> ScriptableRenderPass&;

		SH_RENDER_API auto ReadRenderTextureAsync(RenderTexture& rt, int x, int y) -> std::future<std::unique_ptr<IBuffer>>;

		SH_RENDER_API auto GetSubmittedCommands() const -> const std::vector<SubmittedCommand>& { return submittedCmds; }

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<ScriptableRenderPass, T>>, typename... Args>
		auto AddRenderPass(Args&&... args) -> T&;
	protected:
		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API virtual void Setup(const RenderTarget& data);
		SH_RENDER_API virtual void Execute(const RenderTarget& data);
		SH_RENDER_API void ExecuteTransfer(uint32_t imgIdx);
		SH_RENDER_API void CallReadbacks();

		SH_RENDER_API void EnqueRenderPass(ScriptableRenderPass& pass);
		SH_RENDER_API void ResetSubmittedCommands(IRenderContext& ctx);

		SH_RENDER_API void ResetSwapChainStates();
	private:
		auto BuildBarrierInfo(ScriptableRenderPass& pass, uint32_t imgIdx) -> std::vector<BarrierInfo>;
	protected:
		IRenderContext& ctx;

		std::vector<std::unique_ptr<ScriptableRenderPass>> allPasses;
	private:
		SH_RENDER_API static std::vector<ImageUsage> swapChainStates;

		std::vector<ScriptableRenderPass*> activePasses;
		std::vector<SubmittedCommand> submittedCmds;

		std::unique_ptr<CopyPass> cpyPass;

		struct SyncData
		{
			RenderTexture* src;
			int x;
			int y;
			std::promise<std::unique_ptr<IBuffer>> promise;
		};
		std::vector<SyncData> syncDatas;

		struct PendingReadback
		{
			std::unique_ptr<IBuffer> buffer;
			std::promise<std::unique_ptr<IBuffer>> promise;
		};
		std::vector<PendingReadback> pendingReadbacks;

		bool bSyncDirty = false;
	};

	template<typename T, typename, typename... Args>
	inline auto ScriptableRenderer::AddRenderPass(Args&&... args) -> T&
	{
		std::unique_ptr<T> uniquePtr = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = uniquePtr.get();
		allPasses.push_back(std::move(uniquePtr));
		return *ptr;
	}

	inline void IRenderThrMethod<class ScriptableRenderer>::Setup(ScriptableRenderer& renderer, const RenderTarget& data)
	{
		renderer.Setup(data);
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::Execute(ScriptableRenderer& renderer, const RenderTarget& data)
	{
		renderer.Execute(data);
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::ExecuteTransfer(ScriptableRenderer& renderer, uint32_t imgIdx)
	{
		renderer.ExecuteTransfer(imgIdx);
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::EnqueRenderPass(ScriptableRenderer& renderer, ScriptableRenderPass& pass)
	{
		renderer.EnqueRenderPass(pass);
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::CallReadbacks(ScriptableRenderer& renderer)
	{
		renderer.CallReadbacks();
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::ResetSubmittedCommands(ScriptableRenderer& renderer, IRenderContext& ctx)
	{
		renderer.ResetSubmittedCommands(ctx);
	}
	inline void IRenderThrMethod<class ScriptableRenderer>::ResetSwapChainStates(ScriptableRenderer& renderer)
	{
		renderer.ResetSwapChainStates();
	}
}//namespace