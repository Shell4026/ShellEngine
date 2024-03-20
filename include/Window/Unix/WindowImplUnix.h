#pragma once

#include "../WindowImpl.h"
namespace sh {
	class WindowImplUnix : public WindowImpl
	{
	public:
		~WindowImplUnix() override;

		auto Create(const std::wstring& title, int wsize, int hsize) -> WinHandle override;
		bool ProcessEvent() override;
	};
}