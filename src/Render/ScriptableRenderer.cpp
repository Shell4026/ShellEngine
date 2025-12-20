#include "ScriptableRenderer.h"
#include "RenderTexture.h"
#include "BufferFactory.h"

#include "Core/ThreadPool.h"
#include "Core/ThreadSyncManager.h"

#include <algorithm>
namespace sh::render
{
	ScriptableRenderer::ScriptableRenderer(IRenderContext& ctx) :
		ctx(ctx)
	{
		cpyPass = std::make_unique<CopyPass>();
	}

	SH_RENDER_API auto ScriptableRenderer::AddRenderPass(const core::Name& passName, RenderQueue renderQueue) -> ScriptableRenderPass&
	{
		allPasses.push_back(std::make_unique<ScriptableRenderPass>(passName, renderQueue));
		return *allPasses.back();
	}
	SH_RENDER_API auto ScriptableRenderer::ReadRenderTextureAsync(RenderTexture& rt, int x, int y) -> std::future<std::unique_ptr<IBuffer>>
	{
		SyncDirty();

		SyncData data{};
		data.src = &rt;
		data.x = x;
		data.y = y;

		return syncDatas.emplace_back(std::move(data)).promise.get_future();
	}

	SH_RENDER_API void ScriptableRenderer::SyncDirty()
	{
		if (bSyncDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this);
		bSyncDirty = true;
	}
	SH_RENDER_API void ScriptableRenderer::Sync()
	{
		for (auto& data : syncDatas)
		{
			std::unique_ptr<IBuffer> buffer = BufferFactory::Create(ctx, sizeof(uint8_t) * 4, true);
			IRenderThrMethod<CopyPass>::EnqueCopyImagePixelToBuffer(*cpyPass, *data.src, data.x, data.y, *buffer.get());

			pendingReadbacks.push_back(PendingReadback{ std::move(buffer), std::move(data.promise) });
		}
		syncDatas.clear();
		bSyncDirty = false;
	}

	SH_RENDER_API void ScriptableRenderer::Setup(const RenderTarget& data)
	{
		for (auto& pass : allPasses)
			activePasses.push_back(pass.get());
	}
	SH_RENDER_API void ScriptableRenderer::Execute(const RenderTarget& data)
	{
		std::stable_sort(activePasses.begin(), activePasses.end(),
			[&](const ScriptableRenderPass* left, const ScriptableRenderPass* right)
			{
				return left->renderQueue < right->renderQueue;
			}
		);

		for (auto& pass : activePasses)
			IRenderThrMethod<ScriptableRenderPass>::Configure(*pass, data);

		// 각 패스에서 사용되는 렌더 텍스쳐를 추적하여 커맨드를 기록하기전에 배리어를 걸어줘야 함.
		std::vector<std::vector<BarrierInfo>> barriers(activePasses.size());

		for (int i = 0; i < activePasses.size(); ++i)
		{
			barriers[i] = BuildBarrierInfo(*activePasses[i], data.frameIndex);
		}

		std::vector<std::future<std::pair<ScriptableRenderPass*, CommandBuffer*>>> futurePasses;
		futurePasses.reserve(activePasses.size());

		int idx = 0;
		for (ScriptableRenderPass* pass : activePasses)
		{
			futurePasses.push_back(core::ThreadPool::GetInstance()->AddTask(
				[idx, pass, &ctx = ctx, &data, &barriers]() -> std::pair<ScriptableRenderPass*, CommandBuffer*>
				{
					CommandBuffer* cmd = ctx.AllocateCommandBuffer();
					cmd->Begin(true);
					IRenderThrMethod<ScriptableRenderPass>::EmitBarrier(*pass, *cmd, ctx, barriers[idx]);
					IRenderThrMethod<ScriptableRenderPass>::Record(*pass, *cmd, ctx, data);
					cmd->End();
					return { pass, cmd };
				}
			));
			++idx;
		}
		// 모든 패스가 기록이 될 때 까지 대기
		std::vector<std::pair<ScriptableRenderPass*, CommandBuffer*>> recordedCmds;
		recordedCmds.reserve(futurePasses.size());
		for (auto& futurePass : futurePasses)
			recordedCmds.push_back(futurePass.get());

		for (auto& [pass, cmd] : recordedCmds)
			submittedCmds.push_back({ *pass, *cmd });

		activePasses.clear();
	}
	SH_RENDER_API void ScriptableRenderer::ExecuteTransfer(uint32_t imgIdx)
	{
		RenderTarget target{};
		target.camera = nullptr;
		target.target = nullptr;
		target.drawables = nullptr;
		target.frameIndex = imgIdx;

		IRenderThrMethod<ScriptableRenderPass>::Configure(*cpyPass, target);

		std::vector<BarrierInfo> preBarriers = BuildBarrierInfo(*cpyPass, imgIdx);
		std::vector<BarrierInfo> postBarriers = { BarrierInfo{imgIdx, swapChainStates[imgIdx], ImageUsage::Present} };
		swapChainStates[imgIdx] = ImageUsage::Present;

		RenderTarget rt{};
		rt.frameIndex = imgIdx;
		rt.camera = nullptr;
		rt.target = nullptr;
		rt.drawables = nullptr;

		CommandBuffer* cmd = ctx.AllocateCommandBuffer();
		cmd->Begin(true);
		IRenderThrMethod<ScriptableRenderPass>::EmitBarrier(*cpyPass, *cmd, ctx, preBarriers);
		IRenderThrMethod<ScriptableRenderPass>::Record(*cpyPass, *cmd, ctx, rt);
		IRenderThrMethod<ScriptableRenderPass>::EmitBarrier(*cpyPass, *cmd, ctx, postBarriers);
		cmd->End();

		submittedCmds.push_back({ *cpyPass, *cmd});
	}
	SH_RENDER_API void ScriptableRenderer::CallReadbacks()
	{
		for (auto& p : pendingReadbacks)
			p.promise.set_value(std::move(p.buffer));
		pendingReadbacks.clear();
	}

	SH_RENDER_API void ScriptableRenderer::EnqueRenderPass(ScriptableRenderPass& pass)
	{
		activePasses.push_back(&pass);
	}
	SH_RENDER_API void ScriptableRenderer::ResetSubmittedCommands(IRenderContext& ctx)
	{
		for (auto& submittedCmd : submittedCmds)
			ctx.DeallocateCommandBuffer(submittedCmd.cmd);
		submittedCmds.clear();
	}
	auto ScriptableRenderer::BuildBarrierInfo(ScriptableRenderPass& pass, uint32_t imgIdx) -> std::vector<BarrierInfo>
	{
		std::vector<BarrierInfo> barriers;
		for (auto& [rt, usage] : pass.GetRenderTextures())
		{
			BarrierInfo& barrierInfo = barriers.emplace_back();
			if (rt == nullptr) // 스왑체인인 경우
			{
				barrierInfo.target = imgIdx;
				if (swapChainStates.size() <= imgIdx)
					swapChainStates.resize(imgIdx + 1, ImageUsage::Undefined);

				barrierInfo.lastUsage = swapChainStates[imgIdx];
				swapChainStates[imgIdx] = usage;
			}
			else
			{
				barrierInfo.target = rt;
				barrierInfo.lastUsage = rt->GetUsage();
				IRenderThrMethod<RenderTexture>::ChangeUsage(const_cast<RenderTexture&>(*rt), usage);
			}
			barrierInfo.curUsage = usage;
		}
		return barriers;
	}
}//namespace
