#pragma once

#include "IDrawable.h"
#include "Export.h"
#include "../Window/Window.h"

#include <queue>
namespace sh::render {
	class Renderer {
	protected:
		std::queue<const IDrawable*> drawList;
	public:
		SH_RENDER_API virtual ~Renderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win) = 0;
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render(float deltaTime) = 0;
		SH_RENDER_API virtual void Pause(bool b) = 0;

		SH_RENDER_API virtual bool IsInit() const = 0;

		SH_RENDER_API void ClearDrawList();
		SH_RENDER_API void PushDrawAble(const IDrawable* drawable);
	};
}