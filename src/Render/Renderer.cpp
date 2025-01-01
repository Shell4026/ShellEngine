#include "pch.h"
#include "Renderer.h"
#include "IDrawable.h"

#include "Core/Util.h"
#include "Core/ThreadSyncManager.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer() :
		window(nullptr), 

		viewportStart(0.f), viewportEnd(100.f),
		bPause(false), bDirty(false)
	{
	}

	SH_RENDER_API void Renderer::Clean()
	{
		for (int thr = 0; thr < drawList.size(); ++thr)
			drawList[thr].clear();
		drawCalls.clear();
	}

	SH_RENDER_API bool Renderer::Init(const sh::window::Window& win)
	{
		window = &win;
		return true;
	}

	SH_RENDER_API void Renderer::PushDrawAble(IDrawable* drawable)
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

	SH_RENDER_API void Renderer::ClearDrawList()
	{
		drawList[core::ThreadType::Game].clear();
		SetDirty();
	}

	SH_RENDER_API void Renderer::SetViewport(const glm::vec2& start, const glm::vec2& end)
	{
		viewportStart = start;
		viewportEnd = end;
	}
	SH_RENDER_API auto Renderer::GetViewportStart() const -> const glm::vec2&
	{
		return viewportStart;
	}
	SH_RENDER_API auto Renderer::GetViewportEnd() const -> const glm::vec2&
	{
		return viewportEnd;
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
		while (!drawableQueue.empty())
		{
			IDrawable* drawable = drawableQueue.front();
			drawableQueue.pop();
			if (!core::IsValid(drawable))
				continue;
			if (!drawable->CheckAssetValid())
				continue;

			auto it = drawList[core::ThreadType::Game].find(drawable->GetCamera());
			if (it == drawList[core::ThreadType::Game].end())
			{
				drawList[core::ThreadType::Game].insert({ drawable->GetCamera(), core::SVector<IDrawable*>{drawable} });
			}
			else
			{
				it->second.push_back(drawable);
			}
		}

		drawList[core::ThreadType::Render] = std::move(drawList[core::ThreadType::Game]);
		bDirty = false;
	}
}