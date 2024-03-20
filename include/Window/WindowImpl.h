#pragma once

#include <string>
#include <queue>
#include <Event.h>

namespace sh {
#ifdef _WIN32
	typedef unsigned int WinHandle;
#elif __unix__
	typedef int WinHandle;
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