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

		std::wstring title;

		std::queue<Event> events;

		bool isOpen : 1;
	public:
		Window();
		~Window();

		void Create(const std::wstring& title, int wsize, int hsize);
		bool PollEvent(Event& event);
		bool IsOpen() const;
		void Close();
	};
}