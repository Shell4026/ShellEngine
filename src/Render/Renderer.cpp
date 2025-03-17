#include "Renderer.h"
#include "Drawable.h"

#include "Core/Util.h"
#include "Core/ThreadSyncManager.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer() :
		window(nullptr),
		bPause(false), bDirty(false)
	{
	}

	SH_RENDER_API void Renderer::Clear()
	{
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
		for (auto& lightingPass : lightingPasses)
			lightingPass->ClearDrawable();

		while (!drawableQueue.empty())
		{
			Drawable* drawable = drawableQueue.front();
			drawableQueue.pop();
			if (!core::IsValid(drawable))
				continue;

			for (auto& lightingPass : lightingPasses)
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

		bDirty = false;
	}

	SH_RENDER_API void Renderer::AddRenderPass(std::unique_ptr<ILightingPass>&& renderPass)
	{
		return lightingPasses.push_back(std::move(renderPass));
	}
	SH_RENDER_API void Renderer::ClearRenderPass()
	{
		lightingPasses.clear();
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
}