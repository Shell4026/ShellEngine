#pragma once
#include "Export.h"
#include "../WindowImpl.h"

#include <X11/Xlib.h>
namespace sh::window 
{
	class WindowImplUnix : public WindowImpl
	{
	public:
		SH_WINDOW_API ~WindowImplUnix() override;

		SH_WINDOW_API auto Create(const std::string& title, int wsize, int hsize, uint32_t style) -> WinHandle override;
		SH_WINDOW_API void Close() override;
		SH_WINDOW_API void ProcessEvent() override;

		SH_WINDOW_API auto GetDisplay() -> Display*;
		SH_WINDOW_API void SetTitle(std::string_view title) override;

		SH_WINDOW_API auto GetWidth() const->uint32_t override;
		SH_WINDOW_API auto GetHeight() const->uint32_t override;

		SH_WINDOW_API void StopTimer(uint32_t ms) override;
		SH_WINDOW_API void Resize(int width, int height) override;
	private:
		auto CovertKeyCode(unsigned int keycode) -> Event::KeyType;

		static auto CheckEvent(Display*, XEvent*, XPointer userData) -> int;
	private:
		Display* display;
		int screen;

		::Window win, root;
		XEvent e;
		XIM xim = nullptr;
		XIC xic = nullptr;

		Atom wmDeleteMessage;

		int widthPrevious;
		int heightPrevious;
	};
}