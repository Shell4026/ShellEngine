#pragma once
#include "WindowImpl.h"
#include <memory>

namespace sh {
	class WindowFactory {
	public:
		static auto CreateWindowImpl() -> std::unique_ptr<WindowImpl>;
	};
}