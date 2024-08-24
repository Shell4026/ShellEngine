#include "Renderer.h"
#include "IDrawable.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		window(nullptr),
		isPause(bPause),

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
		if (cam >= camHandles.size())
			return;
		if (camHandles[cam] == nullptr)
			return;

		auto& map = drawList[GAME_THREAD];
		if (auto it = map.find(*camHandles[cam]); it != map.end())
		{
			it->second.push_back(drawable);
		}
	}

	void Renderer::ClearDrawList()
	{
		emptyHandle = std::queue<CameraHandle>{};
		camHandles.clear();
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
		bPause = b;
	}
	auto Renderer::AddCamera(int depth) -> CameraHandle
	{
		CameraHandle id;
		if (emptyHandle.empty())
		{
			id = nextCameraId++;
			Camera cam{ id, depth };
			auto result = drawList[GAME_THREAD].insert({ cam , std::vector<IDrawable*>{} });

			drawListMutex.lock();
			drawList[RENDER_THREAD].insert({ cam, std::vector<IDrawable*>{} });
			drawListMutex.unlock();

			if (drawList[GAME_THREAD].size() == 1)
				mainCamera = id;

			camHandles.push_back(&result.first->first);
		}
		else
		{
			id = emptyHandle.front();
			emptyHandle.pop();
			Camera cam{ id, depth };
			auto result = drawList[GAME_THREAD].insert({ cam , std::vector<IDrawable*>{} });

			drawListMutex.lock();
			drawList[RENDER_THREAD].insert({ cam , std::vector<IDrawable*>{} });
			drawListMutex.unlock();

			if (drawList[GAME_THREAD].size() == 1)
				mainCamera = id;

			camHandles[id] = &result.first->first;
		}
		return id;
	}

	void Renderer::DeleteCamera(CameraHandle cam)
	{
		if (cam >= camHandles.size())
			return;
		if (camHandles[cam] == nullptr)
			return;

		const Camera* camera = camHandles[cam];

		auto it = drawList[GAME_THREAD].find(*camera);
		if (it == drawList[GAME_THREAD].end())
			return;

		emptyHandle.push(it->first.id);
		drawList[GAME_THREAD].erase(it);

		drawListMutex.lock();
		drawList[RENDER_THREAD].erase(*camera);
		drawListMutex.unlock();

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
		if (cam >= camHandles.size())
			return;
		if (camHandles[cam] == nullptr)
			return;

		auto it = drawList[GAME_THREAD].find(*camHandles[cam]);
		if (it == drawList[GAME_THREAD].end())
			return;
		
		auto& vec = it->second;

		Camera tempCam{ it->first.id, depth };

		drawList[GAME_THREAD].erase(it);
		auto result = drawList[GAME_THREAD].insert({ tempCam, std::move(vec) });

		drawListMutex.lock();
		drawList[RENDER_THREAD].erase(*camHandles[cam]);
		drawList[RENDER_THREAD].insert({ tempCam, std::vector<IDrawable*>{} });
		drawListMutex.unlock();

		camHandles[cam] = &result.first->first;
	}

	void Renderer::SyncGameThread()
	{
		std::swap(drawList[GAME_THREAD], drawList[RENDER_THREAD]);
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
}