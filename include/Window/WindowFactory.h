#pragma once
#include "WindowImpl.h"
#include <memory>

namespace sh::window {
	class SH_WINDOW_API WindowFactory {
	public:
		static auto CreateWindowImpl() -> std::unique_ptr<WindowImpl>;
	};
}