#include "Renderer.h"
#include "Drawable.h"

#include "Core/Util.h"
#include "Core/ThreadSyncManager.h"

#include <cassert>
namespace sh::render
{
	void Renderer::SetDrawCallCount(uint32_t drawcall)
	{
		this->drawcall[static_cast<uint32_t>(core::ThreadType::Render)] = drawcall;
		bDrawCallDirty = true;
		SyncDirty();
	}

	Renderer::Renderer() :
		window(nullptr),
		bPause(false), bDirty(false),
		drawcall({ 0, 0 })
	{
	}
	Renderer::~Renderer()
	{
	}

	SH_RENDER_API void Renderer::Clear()
	{
		drawableQueue = std::queue<Drawable*>{};
		cameraQueue = std::queue<CameraProcess>{};
		drawcall = core::SyncArray<uint32_t>{};

		cameras.clear();

		renderPipelines.clear();
		drawCalls.clear();
	}


	SH_RENDER_API bool Renderer::Init(sh::window::Window& win)
	{
		window = &win;
		return true;
	}

	SH_RENDER_API void Renderer::PushDrawAble(Drawable* drawable)
	{
		if (!core::IsValid(drawable))
			return;

		drawableQueue.push(drawable);
		SyncDirty();
	}

	SH_RENDER_API void Renderer::AddDrawCall(const std::function<void()>& func)
	{
		drawCalls.push_back(func);
	}

	SH_RENDER_API auto Renderer::GetWindow() const -> sh::window::Window&
	{
		assert(window);
		return *window;
	}
	SH_RENDER_API void Renderer::Render()
	{
		if (!bFirstRender)
			threadId = std::this_thread::get_id();
		bFirstRender = true;
	}
	SH_RENDER_API void Renderer::Pause(bool b)
	{
		bPause.store(b, std::memory_order::memory_order_release);
	}
	SH_RENDER_API auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}

	SH_RENDER_API void Renderer::SyncDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this);
		bDirty = true;
	}
	SH_RENDER_API void Renderer::Sync()
	{
		for (auto& lightingPass : renderPipelines)
			lightingPass->ClearDrawable();

		while (!drawableQueue.empty())
		{
			Drawable* drawable = drawableQueue.front();
			drawableQueue.pop();
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid())
				continue;

			for (auto& lightingPass : renderPipelines)
				lightingPass->PushDrawable(drawable);
		}
		while (!cameraQueue.empty())
		{
			CameraProcess process = cameraQueue.front();
			cameraQueue.pop();
			if (process.mode == CameraProcess::Mode::Create)
			{
				cameras.insert(process.cameraPtr);
				OnCameraAdded(process.cameraPtr);
			}
			else
			{
				cameras.erase(process.cameraPtr);
				OnCameraRemoved(process.cameraPtr);
			}
		}
		if (bDrawCallDirty)
			std::swap(drawcall[core::ThreadType::Game], drawcall[core::ThreadType::Render]);

		bDirty = false;
		bDrawCallDirty = false;
	}

	SH_RENDER_API auto Renderer::AddRenderPipeline(std::unique_ptr<RenderPipeline>&& renderPipeline) -> RenderPipeline*
	{
		auto ptr = renderPipeline.get();
		renderPipelines.push_back(std::move(renderPipeline));

		return ptr;
	}
	SH_RENDER_API void Renderer::ClearRenderPipeline()
	{
		renderPipelines.clear();
	}
	SH_RENDER_API auto Renderer::GetRenderPipeline(const core::Name& name) const -> RenderPipeline*
	{
		RenderPipeline* renderPipeline = nullptr;
		for (auto& _renderPipeline : renderPipelines)
		{
			if (_renderPipeline->GetPassName() == name)
			{
				renderPipeline = _renderPipeline.get();
				break;
			}
		}
		return renderPipeline;
	}
	SH_RENDER_API auto Renderer::GetRenderPipelines() -> std::vector<std::unique_ptr<RenderPipeline>>&
	{
		return renderPipelines;
	}
	SH_RENDER_API void Renderer::AddCamera(const Camera& camera)
	{
		cameraQueue.push({ &camera, CameraProcess::Mode::Create });
		SyncDirty();
	}
	SH_RENDER_API void Renderer::RemoveCamera(const Camera& camera)
	{
		cameraQueue.push({ &camera, CameraProcess::Mode::Destroy });
		SyncDirty();
	}

	SH_RENDER_API auto Renderer::GetDrawCall(core::ThreadType thread) const -> uint32_t
	{
		return drawcall[static_cast<uint32_t>(thread)];
	}
	SH_RENDER_API auto Renderer::GetThreadId() const -> std::thread::id
	{
		return threadId;
	}
}