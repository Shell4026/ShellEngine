#pragma once

#include "Export.h"

#include "Event.h"

#include <string>
#include <string_view>
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

		SH_WINDOW_API virtual auto Create(const std::string& title, int wsize, int hsize, uint32_t style) -> WinHandle = 0;
		SH_WINDOW_API virtual void Close() = 0;
		SH_WINDOW_API virtual void ProcessEvent() = 0;
		SH_WINDOW_API virtual void SetTitle(std::string_view title) = 0;
		SH_WINDOW_API virtual auto GetWidth() const->uint32_t = 0;
		SH_WINDOW_API virtual auto GetHeight() const->uint32_t = 0;
		/// @brief 일정 시간동안 스레드를 멈추는 함수
		/// @param ms 밀리초
		SH_WINDOW_API virtual void StopTimer(uint32_t ms) = 0;
	};
}