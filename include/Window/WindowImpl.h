#pragma once

namespace sh {
#ifdef _WIN32
	typedef unsigned int WinHandle;
#elif __unix__
	typedef int WinHandle;
#endif
	class WindowImpl {
	public:
		virtual WinHandle Create() = 0;
	};
}