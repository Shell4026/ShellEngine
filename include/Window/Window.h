#pragma once
#include "WindowImpl.h"

#include <memory>
#include <string_view>
#include <queue>
#include <Event.h>

namespace sh {
	class Window {
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