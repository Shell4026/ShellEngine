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
		window = CreateWindowExW(0, className, wtitle.c_str(), style, 0, 0, wsize, hsize, nullptr, nullptr, GetModuleHandleW(nullptr), this);
		return window;
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
		if (!hwnd)
			return 0;

		WindowImplWin32* win = reinterpret_cast<WindowImplWin32*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (win)
		{
			win->ProcessEvents(msg, wParam, lParam);
		}

		switch (msg)
		{
			//USERDATA영역에 이 클래스의 주소를 저장함.
			//msg == WM_CREATE일시 lParam에는 CREATESTRUCT의 정보가 들어가있음.
		case WM_CREATE:
			win = reinterpret_cast<WindowImplWin32*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			win->window = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win));
			break;
		case WM_CLOSE:// 윈도우 닫는 이벤트는 유저쪽에서 처리
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
		//Window
		case WM_CLOSE:
			e.type = Event::EventType::Close;
			PushEvent(e);
			break;
		case WM_MOVE:
			e.type = Event::EventType::Move;
			PushEvent(e);
			break;
		case WM_SETFOCUS:
			e.type = Event::EventType::WindowFocus;
			PushEvent(e);
			break;
		case WM_KILLFOCUS:
			e.type = Event::EventType::WindowFocusOut;
			PushEvent(e);
			break;

		//keyboard
		case WM_KEYDOWN:
			e.type = Event::EventType::KeyDown;
			e.keyType = ConvertKeycode(wParam);
			PushEvent(e);
			break;
		case WM_KEYUP:
			e.type = Event::EventType::KeyUp;
			e.keyType = ConvertKeycode(wParam);
			PushEvent(e);
			break;

		//mouse
		case WM_LBUTTONDOWN:
			e.type = Event::EventType::MousePressed;
			e.mouseType = Event::MouseType::Left;
			PushEvent(e);
			break;
		case WM_RBUTTONDOWN:
			e.type = Event::EventType::MousePressed;
			e.mouseType = Event::MouseType::Right;
			PushEvent(e);
			break;
		case WM_MOUSEWHEEL:
			e.type = Event::EventType::MouseWheelScrolled;
			Event::MouseWheelScrolled::delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / 120.0f;
			PushEvent(e);
			break;
		default:
			break;
		}
	}

	auto WindowImplWin32::ConvertKeycode(WPARAM wParam) -> Event::KeyType
	{
		//0~9
		if(wParam >= 0x30 && wParam <= 0x39)
			return static_cast<Event::KeyType>(wParam - 0x30 + static_cast<int>(Event::KeyType::Num0));

		//F1~F12
		if (wParam >= 0x70 && wParam <= 0x7B)
			return static_cast<Event::KeyType>(wParam - 0x70 + static_cast<int>(Event::KeyType::F1));

		//A~Z
		if (wParam >= 0x41 && wParam <= 0x5A)
			return static_cast<Event::KeyType>(wParam - 0x41 + static_cast<int>(Event::KeyType::A));

		//LShift, RShift, LCtrl, RCtrl, LAlt, RAlt
		if (wParam >= 0xA0 && wParam <= 0xA5)
			return static_cast<Event::KeyType>(wParam - 0xA0 + static_cast<int>(Event::KeyType::LShift));

		//Left Up Right Down
		if (wParam >= 0x25 && wParam <= 0x28)
			return static_cast<Event::KeyType>(wParam - 0x25 + static_cast<int>(Event::KeyType::Left));

		//PageUp PageDown End Home
		if (wParam >= 0x21 && wParam <= 0x24)
			return static_cast<Event::KeyType>(wParam - 0x21 + static_cast<int>(Event::KeyType::PageUp));

		switch (wParam)
		{
		case 0x20:
			return Event::KeyType::Space;
		case 0x8:
			return Event::KeyType::BackSpace;
		case 0xD:
			return Event::KeyType::Enter;
		case 0x9:
			return Event::KeyType::Tab;
		case 0x1B:
			return Event::KeyType::Esc;
		case 0x2D:
			return Event::KeyType::Insert;
		case 0x2E:
			return Event::KeyType::Delete;
		case 0xBB:
			return Event::KeyType::Plus;
		case 0xBC:
			return Event::KeyType::Comma;
		case 0xBD:
			return Event::KeyType::Minus;
		case 0xBE:
			return Event::KeyType::Period;
		case 0xBF:
			return Event::KeyType::Slash;
		case 0xDC:
			return Event::KeyType::BackSlash;
		case 0xDB:
			return Event::KeyType::LBracket;
		case 0xDD:
			return Event::KeyType::RBracket;
		case 0xBA:
			return Event::KeyType::Semicolon;
		case 0xDE:
			return Event::KeyType::Colon;
		case 0x2C:
			return Event::KeyType::Print;
		case 0x91:
			return Event::KeyType::Pause;
		case 0x90:
			return Event::KeyType::NumLock;
		}
		return Event::KeyType::Unknown;
	}
}