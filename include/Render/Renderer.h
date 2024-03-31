#pragma once

namespace sh::render {
	class Renderer {
	public:
		virtual ~Renderer() {};

		virtual bool Init() = 0;
	};
}