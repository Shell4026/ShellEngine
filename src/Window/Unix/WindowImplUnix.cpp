#include "Unix/WindowImplUnix.h"

#include <iostream>

auto sh::WindowImplUnix::Create() -> WinHandle
{
	std::cout << "WindowImplUnix::Create()\n";
	return 0;
}
