#pragma once

#include "Export.h"
#include "../Window/Window.h"
namespace sh::render {
	class SH_RENDER_API IRenderer {
	public:
		virtual ~IRenderer() = default;

		virtual bool Init(sh::window::Window& win) = 0;
		virtual void Clean() = 0;

		virtual bool IsInit() const = 0;
	};
}