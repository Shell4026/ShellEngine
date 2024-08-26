#include "Renderer.h"
#include "IDrawable.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		window(nullptr),

		apiType(api),
		viewportStart(0.f), viewportEnd(100.f),
		mainCamera(0), nextCameraId(0),
		bPause(false)
	{
	}

	bool Renderer::Camera::operator<(const Camera& other) const
	{
		if (depth == other.depth)
			return id < other.id;
		return depth < other.depth;
	}

	bool Renderer::Init(sh::window::Window& win)
	{
		window = &win;
		return true;
	}

	void Renderer::PushDrawAble(IDrawable* drawable, CameraHandle cam)
	{
		if (drawable == nullptr)
			return;
		if (drawList[GAME_THREAD].empty()) //카메라가 없다.
			return;
		if (cam >= camHandles[GAME_THREAD].size())
			return;
		if (camHandles[GAME_THREAD][cam] == nullptr)
			return;

		auto& map = drawList[GAME_THREAD];
		if (auto it = map.find(*camHandles[GAME_THREAD][cam]); it != map.end())
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
		emptyHandle = std::queue<CameraHandle>{};
		camHandles[GAME_THREAD].clear();
		drawList[GAME_THREAD].clear();
		AddCamera(0);
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
	auto Renderer::AddCamera(int depth) -> CameraHandle
	{
		CameraHandle id;
		if (emptyHandle.empty())
		{
			id = nextCameraId++;
			Camera cam{ id, depth };
			auto result = drawList[GAME_THREAD].insert({ cam , std::vector<IDrawable*>{} });

			if (drawList[GAME_THREAD].size() == 1)
				mainCamera = id;

			camHandles[GAME_THREAD].push_back(&result.first->first);
		}
		else
		{
			id = emptyHandle.front();
			emptyHandle.pop();
			Camera cam{ id, depth };
			auto result = drawList[GAME_THREAD].insert({ cam , std::vector<IDrawable*>{} });

			if (drawList[GAME_THREAD].size() == 1)
				mainCamera = id;

			camHandles[GAME_THREAD][id] = &result.first->first;
		}
		return id;
	}

	void Renderer::DeleteCamera(CameraHandle cam)
	{
		if (cam >= camHandles[GAME_THREAD].size())
			return;
		if (camHandles[GAME_THREAD][cam] == nullptr)
			return;

		const Camera* camera = camHandles[GAME_THREAD][cam];

		auto it = drawList[GAME_THREAD].find(*camera);
		if (it == drawList[GAME_THREAD].end())
			return;

		emptyHandle.push(it->first.id);
		drawList[GAME_THREAD].erase(it);

		if (mainCamera == cam)
		{
			if (drawList[GAME_THREAD].empty())
				return;
			mainCamera = drawList[GAME_THREAD].begin()->first.id;
		}
	}

	void Renderer::SetMainCamera(CameraHandle cam)
	{
		mainCamera = cam;
	}

	void Renderer::SetCameraDepth(CameraHandle cam, int depth)
	{
		if (cam >= camHandles[GAME_THREAD].size())
			return;
		if (camHandles[GAME_THREAD][cam] == nullptr)
			return;

		auto it = drawList[GAME_THREAD].find(*camHandles[GAME_THREAD][cam]);
		if (it == drawList[GAME_THREAD].end())
		{
			return;
		}
		
		auto vec = std::move(it->second);

		Camera tempCam{ it->first.id, depth };

		drawList[GAME_THREAD].erase(it);
		auto result = drawList[GAME_THREAD].insert({ tempCam, std::move(vec) });

		camHandles[GAME_THREAD][cam] = &result.first->first;
	}

	void Renderer::SyncGameThread()
	{
		drawList[RENDER_THREAD] = drawList[GAME_THREAD];
		camHandles[RENDER_THREAD] = camHandles[GAME_THREAD];
		for (auto& pair : drawList[GAME_THREAD])
		{
			pair.second.clear();
		}
		for (auto& pair : drawList[RENDER_THREAD])
		{
			for (auto drawable : pair.second)
			{
				drawable->SyncGameThread();
			}
		}
	}

	auto Renderer::IsPause() const -> bool
	{
		return bPause.load(std::memory_order::memory_order_acquire);
	}
}