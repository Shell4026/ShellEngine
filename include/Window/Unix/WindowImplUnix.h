#pragma once

#include "../WindowImpl.h"
#include <X11/Xlib.h>
namespace sh {
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
	public:
		~WindowImplUnix() override;

		auto Create(const std::string& title, int wsize, int hsize) -> WinHandle override;
		void Close() override;
		void ProcessEvent() override;
	};
}