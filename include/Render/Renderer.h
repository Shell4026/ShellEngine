#pragma once

#include "Export.h"
namespace sh::render {
	class SH_RENDER_API Renderer {
	public:
		virtual ~Renderer() {};

		virtual bool Init() = 0;
	};
}