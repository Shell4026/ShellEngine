#include "Renderer.h"
#include "Drawable.h"

#include "Core/ThreadSyncManager.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer() :
		window(nullptr),
		bPause(false), bDirty(false),
		drawcall({ 0, 0 })
	{
	}
	Renderer::~Renderer()
	{
	}

	SH_RENDER_API void Renderer::SyncDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this, SYNC_PRIORITY);
		bDirty = true;
	}
	SH_RENDER_API void Renderer::Sync()
	{
		if (bDrawCallDirty)
			drawcall[core::ThreadType::Game] = drawcall[core::ThreadType::Render];

		bDirty = false;
		bDrawCallDirty = false;
	}

	SH_RENDER_API bool Renderer::Init(sh::window::Window& win)
	{
		window = &win;
		return true;
	}
	SH_RENDER_API void Renderer::Clear()
	{
		renderer = nullptr;
		renderCommands.Clear();
		drawables.clear();
		drawcall = core::SyncArray<uint32_t>{};

		cameras.clear();

		drawCalls.clear();
	}

	SH_RENDER_API void Renderer::Render()
	{
		if (!bFirstRender)
			threadId = std::this_thread::get_id();
		bFirstRender = true;

		drawables.clear();
		DrainRenderCommands();
	}
	SH_RENDER_API void Renderer::Pause(bool b)
	{
		bPause.store(b, std::memory_order::memory_order_release);
	}

	SH_RENDER_API void Renderer::PushDrawAble(Drawable* drawable)
	{
		if (!core::IsValid(drawable))
			return;

		RenderCommand cmd{};
		cmd.type = RenderCommand::Type::PushDrawable;
		cmd.data = drawable;
		renderCommands.Push(std::move(cmd));
	}

	SH_RENDER_API auto Renderer::GetWindow() const -> sh::window::Window&
	{
		assert(window);
		return *window;
	}

	SH_RENDER_API auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}

	SH_RENDER_API void Renderer::SetScriptableRenderer(ScriptableRenderer& renderer)
	{
		this->renderer = &renderer;
	}
	SH_RENDER_API void Renderer::AddCamera(const Camera& camera)
	{
		RenderCommand cmd{};
		cmd.type = RenderCommand::Type::AddCamera;
		cmd.data = &camera;
		renderCommands.Push(std::move(cmd));
	}
	SH_RENDER_API void Renderer::RemoveCamera(const Camera& camera)
	{
		RenderCommand cmd{};
		cmd.type = RenderCommand::Type::RemoveCamera;
		cmd.data = &camera;
		renderCommands.Push(std::move(cmd));
	}

	SH_RENDER_API auto Renderer::GetDrawCall(core::ThreadType thread) const -> uint32_t
	{
		return drawcall[static_cast<uint32_t>(thread)];
	}
	SH_RENDER_API auto Renderer::GetThreadId() const -> std::thread::id
	{
		return threadId;
	}

	SH_RENDER_API void Renderer::SetDrawCallCount(uint32_t drawcall)
	{
		this->drawcall[static_cast<uint32_t>(core::ThreadType::Render)] = drawcall;
		bDrawCallDirty = true;
		SyncDirty();
	}

	SH_RENDER_API void Renderer::DrainRenderCommands()
	{
		renderCommands.Drain(
			[this](RenderCommand& cmd)
			{
				switch (cmd.type)
				{
				case RenderCommand::Type::PushDrawable:
				{
					Drawable* drawable = std::get<0>(cmd.data).Get();
					if (!core::IsValid(drawable) || !drawable->CheckAssetValid())
						return;
					drawables.push_back(drawable);
					break;
				}
				case RenderCommand::Type::AddCamera:
				{
					const Camera* camera = std::get<1>(cmd.data);
					if (camera == nullptr)
						return;
					auto [it, inserted] = cameras.insert(camera);
					if (inserted)
						OnCameraAdded(camera);
					break;
				}
				case RenderCommand::Type::RemoveCamera:
				{
					const Camera* camera = std::get<1>(cmd.data);
					if (camera == nullptr)
						return;
					if (cameras.erase(camera) != 0)
						OnCameraRemoved(camera);
					break;
				}
				}
			}
		);
	}
}//namespace