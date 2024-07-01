#pragma once

#include "Export.h"

#include "../Window/Window.h"

#include "glm/vec2.hpp"

#include <map>
#include <queue>
#include <set>
#include <utility>
#include <mutex>
namespace sh::render {
	enum class RenderAPI
	{
		OpenGL,
		Vulkan
	};
	class IDrawable;
	class Framebuffer;

	class Renderer {
	public:
		using CameraHandle = uint32_t;
		struct Camera
		{
			CameraHandle id;
			int depth;

			bool operator<(const Camera& other) const;
		};
	private:
		sh::window::Window* window;
		uint32_t nextCameraId;
	protected:
		std::queue<CameraHandle> emptyHandle;
		std::vector<const Camera*> camHandles;
		std::map<Camera, std::queue<IDrawable*>> drawList;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;

		CameraHandle mainCamera;

		bool bPause;
	public:
		const RenderAPI apiType;
		const bool& isPause;
	public:
		SH_RENDER_API Renderer(RenderAPI api);
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b);

		SH_RENDER_API virtual bool IsInit() const = 0;
		SH_RENDER_API virtual auto GetMainFramebuffer() const -> const Framebuffer* = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		SH_RENDER_API virtual void SetViewport(const glm::vec2& start, const glm::vec2& end) = 0;
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2&;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2&;

		SH_RENDER_API void ClearDrawList();
		SH_RENDER_API void PushDrawAble(IDrawable* drawable, CameraHandle camHandle = 0);

		SH_RENDER_API auto GetWindow() -> sh::window::Window&;

		SH_RENDER_API auto AddCamera(int depth = 0) -> CameraHandle;
		SH_RENDER_API void DeleteCamera(CameraHandle cam);
		SH_RENDER_API void SetMainCamera(CameraHandle cam);
		SH_RENDER_API void SetCameraDepth(CameraHandle, int depth);
	};
}