#include "Renderer.h"
#include "Drawable.h"

#include "Core/Util.h"
#include "Core/ThreadSyncManager.h"

#include <cassert>
namespace sh::render
{
	void Renderer::SetDrawCall(uint32_t drawcall)
	{
		this->drawcall[static_cast<uint32_t>(core::ThreadType::Render)] = drawcall;
		SetDirty();
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
		renderPipelines.clear();
		drawCalls.clear();
	}


	SH_RENDER_API bool Renderer::Init(const sh::window::Window& win)
	{
		window = &win;
		return true;
	}

	SH_RENDER_API void Renderer::PushDrawAble(Drawable* drawable)
	{
		if (!core::IsValid(drawable))
			return;

		drawableQueue.push(drawable);
		SetDirty();
	}

	SH_RENDER_API void Renderer::AddDrawCall(const std::function<void()>& func)
	{
		drawCalls.push_back(func);
	}

	SH_RENDER_API auto Renderer::GetWindow() const -> const sh::window::Window&
	{
		assert(window);
		return *window;
	}

	SH_RENDER_API void Renderer::Pause(bool b)
	{
		bPause.store(b, std::memory_order::memory_order_release);
	}
	SH_RENDER_API auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}

	SH_RENDER_API void Renderer::SetDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);
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
		std::swap(drawcall[core::ThreadType::Game], drawcall[core::ThreadType::Render]);

		bDirty = false;
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
		auto it = std::find_if(renderPipelines.begin(), renderPipelines.end(), [&](const std::unique_ptr<RenderPipeline>& pipeline)
			{
				return pipeline->GetPassName() == name;
			}
		);
		if (it == renderPipelines.end())
			return nullptr;
		return it->get();
	}

	SH_RENDER_API void Renderer::AddCamera(const Camera& camera)
	{
		cameraQueue.push({ &camera, CameraProcess::Mode::Create });
		SetDirty();
	}
	SH_RENDER_API void Renderer::RemoveCamera(const Camera& camera)
	{
		cameraQueue.push({ &camera, CameraProcess::Mode::Destroy });
		SetDirty();
	}

	SH_RENDER_API auto Renderer::GetDrawCall(core::ThreadType thread) const -> uint32_t
	{
		return drawcall[static_cast<uint32_t>(thread)];
	}
}