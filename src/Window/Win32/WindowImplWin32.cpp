#include "Win32/WindowImplWin32.h"

#include <iostream>
namespace sh {
	WindowImplWin32::WindowImplWin32() :
		className(L"ShellEngine Window"),
		window{ 0 }
	{
	}

	WindowImplWin32::~WindowImplWin32()
	{
		DestroyWindow(window);
		UnregisterClassW(className, GetModuleHandle(nullptr));
	}

	WinHandle WindowImplWin32::Create(const std::wstring& title, int wsize, int hsize)
	{
		std::cout << "WindowImplWin32::Create()\n";
		WNDCLASSW wc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &WindowImplWin32::EventHandler;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandleW(nullptr);
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = className;

		RegisterClassW(&wc);

		unsigned long style = WS_VISIBLE | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
		window = CreateWindowW(className, title.c_str(), style, 0, 0, wsize, hsize, nullptr, nullptr, GetModuleHandleW(nullptr), this);
		return 0;
	}

	void WindowImplWin32::Close()
	{
		CloseWindow(window);
	}

	void WindowImplWin32::ProcessEvent()
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT WindowImplWin32::EventHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowImplWin32* win = hwnd ? reinterpret_cast<WindowImplWin32*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)) : nullptr;
		if (win)
		{
			win->ProcessEvents(msg, wParam, lParam);
		}

		switch (msg)
		{
		case WM_CREATE:
			win = reinterpret_cast<WindowImplWin32*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			win->window = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win));
			break;
		case WM_CLOSE:
			return 0;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void WindowImplWin32::ProcessEvents(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			Event e;
			e.type = Event::EventType::Close;
			PushEvent(e);
			break;
		}
	}
}