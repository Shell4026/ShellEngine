#include "Renderer.h"

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
		if (drawList.empty()) //카메라가 없다.
			return;
		if (cam >= camHandles.size())
			return;
		if (camHandles[cam] == nullptr)
			return;

		if (auto it = drawList.find(*camHandles[cam]); it != drawList.end())
		{
			it->second.push(drawable);
		}
	}

	void Renderer::ClearDrawList()
	{
		emptyHandle = std::queue<CameraHandle>{};
		camHandles.clear();
		drawList.clear();
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
			auto result = drawList.insert({cam , std::queue<IDrawable*>{} });
			if (drawList.size() == 1)
				mainCamera = id;

			camHandles.push_back(&result.first->first);
		}
		else
		{
			id = emptyHandle.front();
			emptyHandle.pop();
			Camera cam{ id, depth };
			auto result = drawList.insert({ cam , std::queue<IDrawable*>{} });
			if (drawList.size() == 1)
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

		auto it = drawList.find(*camera);
		if (it == drawList.end())
			return;

		emptyHandle.push(it->first.id);
		drawList.erase(it);

		if (mainCamera == cam)
		{
			if (drawList.empty())
				return;
			mainCamera = drawList.begin()->first.id;
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

		auto it = drawList.find(*camHandles[cam]);
		if (it == drawList.end())
			return;
		
		auto queue = std::move(it->second);

		Camera tempCam{ it->first.id, depth };

		drawList.erase(it);
		auto result = drawList.insert({ tempCam, std::move(queue) });
		camHandles[cam] = &result.first->first;
	}
}