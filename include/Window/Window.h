#pragma once
#include "WindowImpl.h"
#include <memory>

namespace sh {
	class Window {
	private:
		std::unique_ptr<WindowImpl> winImpl;
	public:
		void Create();
	};
}