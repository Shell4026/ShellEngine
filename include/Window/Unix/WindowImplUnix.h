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

		int widthPrevious;
		int heightPrevious;
	private:
		auto CovertKeyCode(unsigned int keycode) -> Event::KeyType;

		static auto CheckEvent(Display*, XEvent*, XPointer userData) -> int;
	public:
		~WindowImplUnix() override;

		auto Create(const std::string& title, int wsize, int hsize, uint32_t style) -> WinHandle override;
		void Close() override;
		void ProcessEvent() override;

		auto GetDisplay() -> Display*;
		void SetTitle(std::string_view title) override;

		auto GetWidth() const->uint32_t override;
		auto GetHeight() const->uint32_t override;

		void StopTimer(uint32_t ms) override;
	};
}