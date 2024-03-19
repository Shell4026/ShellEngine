#include "WindowFactory.h"

#ifdef _WIN32
#include "Win32/WindowImplWin32.h"
#elif __unix__
#include "Unix/WindowImplUnix.h"
#endif

auto sh::WindowFactory::CreateWindowImpl() -> std::unique_ptr<WindowImpl>
{
#ifdef _WIN32
	return std::make_unique<WindowImplWin32>();
#elif __unix__
	return std::make_unique<WindowImplUnix>();
#endif
}