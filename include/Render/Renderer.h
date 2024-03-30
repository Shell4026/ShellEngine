#pragma once

namespace sh::render {
	class Renderer {
	public:
		virtual ~Renderer() {};

		virtual void Init() = 0;
	};
}