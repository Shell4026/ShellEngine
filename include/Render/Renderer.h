#pragma once

#include "Export.h"
#include "../Window/Window.h"

#include "glm/vec2.hpp"

#include <queue>
namespace sh::render {
	enum class RenderAPI
	{
		OpenGL,
		Vulkan
	};
	class IDrawable;
	class Framebuffer;

	class Renderer {
	protected:
		std::queue<IDrawable*> drawList;
	public:
		const RenderAPI apiType;

		glm::vec2 viewportPos;
		glm::vec2 viewportSize;
	public:
		SH_RENDER_API Renderer(RenderAPI api);
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win) = 0;
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b) = 0;

		SH_RENDER_API virtual bool IsInit() const = 0;
		SH_RENDER_API virtual auto GetMainFramebuffer() const -> const Framebuffer* = 0;

		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;

		SH_RENDER_API void ClearDrawList();
		SH_RENDER_API void PushDrawAble(IDrawable* drawable);
	};
}