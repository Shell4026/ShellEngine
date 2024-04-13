#pragma once
#pragma warning(disable: 4251)
#include "WindowImpl.h"

#include "Event.h"

#include <memory>
#include <string_view>
#include <queue>

namespace sh::window {
	class SH_WINDOW_API Window {
	private:
		std::unique_ptr<WindowImpl> winImpl;
		WinHandle handle;

		std::string title;

		std::queue<Event> events;

		bool isOpen : 1;
	public:
		Window();
		~Window();

		void Create(const std::string& title, int wsize, int hsize);
		bool PollEvent(Event& event);
		bool IsOpen() const;
		void Close();

		auto GetNativeHandle()->WinHandle;
	};
}