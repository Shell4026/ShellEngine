#include "pch.h"
#include "Renderer.h"
#include "IDrawable.h"

#include "Core/Util.h"
#include "Core/GarbageCollection.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer(RenderAPI api, core::ThreadSyncManager& syncManager) :
		window(nullptr), syncManager(syncManager),

		apiType(api),
		viewportStart(0.f), viewportEnd(100.f),
		bPause(false), bDirty(false)
	{
	}
	void Renderer::Clean()
	{
		drawList[core::ThreadType::Game].clear();
		drawList[core::ThreadType::Render].clear();
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

		core::GarbageCollection::GetInstance()->SetRootSet(drawable);

		auto it = drawList[core::ThreadType::Game].find(drawable->GetCamera());
		if (it == drawList[core::ThreadType::Game].end())
		{
			drawList[core::ThreadType::Game].insert({ drawable->GetCamera(), core::SVector<IDrawable*>{drawable} });
		}
		else
		{
			it->second.push_back(drawable);
		}
		SetDirty();
	}

	void Renderer::AddDrawCall(const std::function<void()>& func)
	{
		drawCalls.push_back(func);
	}

	void Renderer::ClearDrawList()
	{
		drawList[core::ThreadType::Game].clear();
		SetDirty();
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

	void Renderer::SetDirty()
	{
		if (bDirty)
			return;

		syncManager.PushSyncable(*this);
		bDirty = true;
	}
	void Renderer::Sync()
	{
		auto gc = core::GarbageCollection::GetInstance();
		for (auto& [_, drawVec] : drawList[core::ThreadType::Render])
		{
			for (IDrawable* drawable : drawVec)
				gc->RemoveRootSet(drawable);
		}
		drawList[core::ThreadType::Render] = std::move(drawList[core::ThreadType::Game]);
		bDirty = false;
	}

	auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}

	auto Renderer::GetThreadSyncManager() const -> core::ThreadSyncManager&
	{
		return syncManager;
	}
}