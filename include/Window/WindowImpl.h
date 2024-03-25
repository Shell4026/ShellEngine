#pragma once

#include <string>
#include <queue>
#include <Event.h>

#ifdef _WIN32
struct HWND__;
#endif

namespace sh {
#ifdef _WIN32
	using WinHandle = HWND__*;
#elif __unix__
	using WinHandle = long long;
#endif

	class WindowImpl {
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