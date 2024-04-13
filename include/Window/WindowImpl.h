#pragma once

#include "Export.h"

#include "Event.h"

#include <string>
#include <queue>

#ifdef _WIN32
struct HWND__;
#elif __linux__
#include <utility>

struct _XDisplay;
using XWindow = unsigned long;
#endif

namespace sh::window {
#ifdef _WIN32
	using WinHandle = HWND__*;
#elif __linux__
	using WinHandle = std::pair<_XDisplay*, XWindow>;
#endif

	class SH_WINDOW_API WindowImpl {
	private:
		std::queue<Event> events;
	public:
		virtual ~WindowImpl();

		void PushEvent(const Event& e);
		Event PopEvent();
		bool IsEmptyEvent() const;

		virtual auto Create(const std::string& title, int wsize, int hsize) -> WinHandle = 0;
		virtual void Close() = 0;
		virtual void ProcessEvent() = 0;
	};
}