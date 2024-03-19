#pragma once

#include "../WindowImpl.h"

namespace sh {
	class WindowImplWin32 : public WindowImpl {
	public:
		auto Create() -> WinHandle override;
	};
}