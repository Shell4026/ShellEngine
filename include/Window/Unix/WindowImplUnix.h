#pragma once

#include "../WindowImpl.h"

#include <X11/Xlib.h>
namespace sh::window {
	class WindowImplUnix : public WindowImpl
	{
	private:
		Display* display;
		int screen;

		::Window win, root;
		XEvent e;

		Atom wmDeleteMessage;
	private:
		auto CovertKeyCode(unsigned int keycode) -> Event::KeyType;

		static auto CheckEvent(Display*, XEvent*, XPointer userData) -> int;
	public:
		~WindowImplUnix() override;

		auto Create(const std::string& title, int wsize, int hsize) -> WinHandle override;
		void Close() override;
		void ProcessEvent() override;

		auto GetDisplay() -> Display*;
		void SetTitle(std::string_view title) override;
	};
}