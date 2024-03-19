#pragma once

#include "../WindowImpl.h"
namespace sh {
	class WindowImplUnix : public WindowImpl
	{
	public:
		auto Create() -> WinHandle override;
	};
}