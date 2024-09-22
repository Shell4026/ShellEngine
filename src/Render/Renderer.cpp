#include "Renderer.h"
#include "IDrawable.h"

#include "Core/Util.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		window(nullptr),

		apiType(api),
		viewportStart(0.f), viewportEnd(100.f),
		bPause(false)
	{
	}
	void Renderer::Clean()
	{
		drawList[GAME_THREAD].clear();
		drawList[RENDER_THREAD].clear();
		drawCalls.clear();
	}

	bool Renderer::Init(sh::window::Window& win)
	{
		window = &win;
		return true;
	}

	void Renderer::PushDrawAble(IDrawable* drawable)
	{
		if (!core::IsValid(drawable))
			return;

		auto it = drawList[GAME_THREAD].find(drawable->GetCamera());
		if (it == drawList[GAME_THREAD].end())
		{
			drawList[GAME_THREAD].insert({ drawable->GetCamera(), core::SVector<IDrawable*>{drawable} });
		}
		else
		{
			it->second.push_back(drawable);
		}
	}

	void Renderer::AddDrawCall(const std::function<void()>& func)
	{
		drawCalls.push_back(func);
	}

	void Renderer::ClearDrawList()
	{
		drawList[GAME_THREAD].clear();
	}

	auto Renderer::GetViewportStart() const -> const glm::vec2&
	{
		return viewportStart;
	}
	auto Renderer::GetViewportEnd() const -> const glm::vec2&
	{
		return viewportEnd;
	}
	auto Renderer::GetWindow() -> sh::window::Window&
	{
		assert(window);
		return *window;
	}
	void Renderer::Pause(bool b)
	{
		bPause.store(b, std::memory_order::memory_order_release);
	}

	void Renderer::SyncGameThread()
	{
		drawList[RENDER_THREAD] = std::move(drawList[GAME_THREAD]);
		drawList[GAME_THREAD].clear();
		while (!syncQueue.empty())
		{
			core::ISyncable* syncObj = syncQueue.front();
			syncQueue.pop();

			syncObj->Sync();
		}
	}

	auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}

	void Renderer::PushSyncObject(core::ISyncable& syncObj)
	{
		syncQueue.push(&syncObj);
	}
}