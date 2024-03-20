#include "Unix/WindowImplUnix.h"

#include <iostream>

namespace sh {
	WindowImplUnix::~WindowImplUnix()
	{
	}

	auto WindowImplUnix::Create(const std::wstring& title, int wsize, int hsize) -> WinHandle
	{
		std::cout << "WindowImplUnix::Create()\n";
		return 0;
	}

	bool WindowImplUnix::ProcessEvent()
	{
		return true;
	}
}