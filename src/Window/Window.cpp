#include "Window.h"

#include "WindowFactory.h"
#include "WindowImpl.h"

#include <iostream>

namespace sh {
	void Window::Create()
	{
		std::cout << "Init\n";

		winImpl = WindowFactory::CreateWindowImpl();
		winImpl->Create();
	}
}
