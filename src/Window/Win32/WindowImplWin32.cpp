#include "Win32/WindowImplWin32.h"

#include <iostream>
namespace sh {
	WinHandle WindowImplWin32::Create()
	{
		std::cout << "WindowImplWin32::Create()\n";
		return 0;
	}
}