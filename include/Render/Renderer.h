#pragma once

#include "Export.h"
#include "../Window/Window.h"
namespace sh::render {
	class IRenderer {
	public:
		SH_RENDER_API virtual ~IRenderer() = default;

		SH_RENDER_API virtual bool Init(sh::window::Window& win) = 0;
		SH_RENDER_API virtual bool Resizing() = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Render() = 0;
		SH_RENDER_API virtual void Pause(bool b) = 0;

		SH_RENDER_API virtual bool IsInit() const = 0;
	};
}