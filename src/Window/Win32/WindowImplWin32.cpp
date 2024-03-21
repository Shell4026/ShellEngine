#include "Win32/WindowImplWin32.h"

#include <iostream>
#include <../Core/Util.h>
namespace sh {
	WindowImplWin32::WindowImplWin32() :
		className(L"ShellEngine Window"),
		window{ 0 }
	{
	}

	WindowImplWin32::~WindowImplWin32()
	{
		Close();
		DestroyWindow(window);
		UnregisterClassW(className, GetModuleHandle(nullptr));
	}

	void WindowImplWin32::RegisterWindow()
	{
		WNDCLASSW wc{ 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &WindowImplWin32::EventHandler;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandleW(nullptr); //현재 실행중인 프로세스의 핸들
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = className;

		RegisterClassW(&wc);
	}

	WinHandle WindowImplWin32::Create(const std::string& title, int wsize, int hsize)
	{
		std::cout << "WindowImplWin32::Create()\n";
		RegisterWindow();

		unsigned long style = WS_VISIBLE | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
		std::wstring wtitle = sh::Util::U8StringToWstring(title);
		window = CreateWindowW(className, wtitle.c_str(), style, 0, 0, wsize, hsize, nullptr, nullptr, GetModuleHandleW(nullptr), this);
		return 0;
	}

	void WindowImplWin32::Close()
	{
		CloseWindow(window);
	}


	void WindowImplWin32::ProcessEvent()
	{
		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
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
			//USERDATA영역에 이 클래스의 주소를 저장함.
			//msg == WM_CREATE일시 lParam에는 CREATESTRUCT의 정보가 들어가있음.
			win = reinterpret_cast<WindowImplWin32*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			win->window = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win));
			break;
		case WM_CLOSE:
			return 0;
		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void WindowImplWin32::ProcessEvents(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//EventHandler -> ProcessEvents로 호출
		switch (msg)
		{
			Event e;
		case WM_CLOSE:
			e.type = Event::EventType::Close;
			PushEvent(e);
			break;
		case WM_MOVE:
			e.type = Event::EventType::Move;
			PushEvent(e);
			break;
		}
	}
}