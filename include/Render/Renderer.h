#pragma once

#include "Export.h"
#include "../Window/Window.h"

#include "glm/vec2.hpp"

#include <queue>
#include <utility>
namespace sh::render {
	enum class RenderAPI
	{
		OpenGL,
		Vulkan
	};
	class IDrawable;
	class Framebuffer;

	class Renderer {
	private:
		sh::window::Window* window;
	protected:
		std::queue<IDrawable*> drawList;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;
	public:
		const RenderAPI apiType;
	public:
		SH_RENDER_API Renderer(RenderAPI api);
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win);
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b) = 0;

		SH_RENDER_API virtual bool IsInit() const = 0;
		SH_RENDER_API virtual auto GetMainFramebuffer() const -> const Framebuffer* = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		SH_RENDER_API virtual void SetViewport(const glm::vec2& start, const glm::vec2& end) = 0;
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2&;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2&;

		SH_RENDER_API void ClearDrawList();
		SH_RENDER_API void PushDrawAble(IDrawable* drawable);

		SH_RENDER_API auto GetWindow() -> sh::window::Window&;
	};
}