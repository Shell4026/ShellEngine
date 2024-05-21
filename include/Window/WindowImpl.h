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

	class WindowImpl {
	private:
		std::queue<Event> events;
	public:
		SH_WINDOW_API virtual ~WindowImpl();

		SH_WINDOW_API void PushEvent(const Event& e);
		SH_WINDOW_API Event PopEvent();
		SH_WINDOW_API bool IsEmptyEvent() const;

		SH_WINDOW_API virtual auto Create(const std::string& title, int wsize, int hsize) -> WinHandle = 0;
		SH_WINDOW_API virtual void Close() = 0;
		SH_WINDOW_API virtual void ProcessEvent() = 0;
		SH_WINDOW_API virtual void SetTitle(std::string_view title) = 0;
	};
}