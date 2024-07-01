#include "Win32/WindowImplWin32.h"
#include "Window.h"

#include <../Core/Util.h>

#include <iostream>

namespace sh::window {
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

	WinHandle WindowImplWin32::Create(const std::string& title, int wsize, int hsize, uint32_t style)
	{
		std::cout << "WindowImplWin32::Create()\n";
		RegisterWindow();

		unsigned long winStyle = WS_VISIBLE | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
		if ((style & Window::Style::Resize) == 0b0001)
			winStyle |= WS_THICKFRAME;
		std::wstring wtitle = sh::core::Util::U8StringToWstring(title);
		window = CreateWindowExW(0, className, wtitle.c_str(), winStyle, 0, 0, wsize, hsize, nullptr, nullptr, GetModuleHandleW(nullptr), this);
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

		//USERDATA영역에 이 클래스의 주소를 저장함.
		//msg == WM_CREATE일시 lParam에는 CREATESTRUCT의 정보가 들어가있음.
		//CreateWindowExW의 가장 마지막 파라미터에 클래스의 주소를 전달했기 때문
		if (msg == WM_CREATE)
		{
			auto win = reinterpret_cast<WindowImplWin32*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			win->window = hwnd;

			//이벤트를 전달한 창의 USERDATA의 값을 수정
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win));
		}

		//이벤트를 전달한 창의 USERDATA값을 가져온다.
		WindowImplWin32* win = reinterpret_cast<WindowImplWin32*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (win)
		{
			win->ProcessEvents(msg, wParam, lParam);
		}

		// 윈도우 닫는 이벤트는 유저쪽에서 처리
		if (msg == WM_CLOSE)
			return 0;

		//Alt키나 f10키 이벤트는 유저가 처리해야 함
		if(msg == WM_SYSCOMMAND)
			if (wParam == SC_KEYMENU)
				return 0;

		//기본 커널 처리
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void WindowImplWin32::ProcessEvents(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//EventHandler -> ProcessEvents로 호출
		
		if (window == nullptr)
			return;

		//std::cout << "event: " << std::hex << msg << '\n';

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
		case WM_SIZE:
		case WM_SIZING:
			e.type = Event::EventType::Resize;
			PushEvent(e);
			break;
		//keyboard
		case WM_SYSKEYDOWN: //alt, f10
		case WM_KEYDOWN:
			e.type = Event::EventType::KeyDown;
			e.keyType = ConvertKeycode(wParam);
			PushEvent(e);
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			e.type = Event::EventType::KeyUp;
			e.keyType = ConvertKeycode(wParam);
			PushEvent(e);
			break;

		//mouse
		case WM_MOUSEMOVE:
		{
			const int x = static_cast<std::int16_t>(LOWORD(lParam));
			const int y = static_cast<std::int16_t>(HIWORD(lParam));

			Event::MousePosition::mouseX = x;
			Event::MousePosition::mouseY = y;

			e.type = Event::EventType::MouseMove;
			PushEvent(e);
			break;
		}
		case WM_LBUTTONDOWN:
			e.type = Event::EventType::MousePressed;
			e.mouseType = Event::MouseType::Left;
			PushEvent(e);
			break;
		case WM_LBUTTONUP:
			e.type = Event::EventType::MouseReleased;
			e.mouseType = Event::MouseType::Left;
			PushEvent(e);
			break;
		case WM_RBUTTONDOWN:
			e.type = Event::EventType::MousePressed;
			e.mouseType = Event::MouseType::Right;
			PushEvent(e);
			break;
		case WM_RBUTTONUP:
			e.type = Event::EventType::MouseReleased;
			e.mouseType = Event::MouseType::Right;
			PushEvent(e);
			break;
		case WM_MBUTTONDOWN:
			e.type = Event::EventType::MousePressed;
			e.mouseType = Event::MouseType::Middle;
			PushEvent(e);
			break;
		case WM_MBUTTONUP:
			e.type = Event::EventType::MouseReleased;
			e.mouseType = Event::MouseType::Middle;
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
		//std::cout << std::hex << wParam << "\n";

		//0~9
		if(wParam >= 0x30 && wParam <= 0x39)
			return static_cast<Event::KeyType>(wParam - 0x30 + static_cast<int>(Event::KeyType::Num0));

		//F1~F12
		if (wParam >= VK_F1 && wParam <= VK_F12)
			return static_cast<Event::KeyType>(wParam - VK_F1 + static_cast<int>(Event::KeyType::F1));

		//A~Z
		if (wParam >= 0x41 && wParam <= 0x5A)
			return static_cast<Event::KeyType>(wParam - 0x41 + static_cast<int>(Event::KeyType::A));

		//Left Up Right Down
		if (wParam >= VK_LEFT && wParam <= VK_DOWN)
			return static_cast<Event::KeyType>(wParam - VK_LEFT + static_cast<int>(Event::KeyType::Left));

		//PageUp PageDown End Home
		if (wParam >= VK_PRIOR && wParam <= VK_HOME)
			return static_cast<Event::KeyType>(wParam - VK_PRIOR + static_cast<int>(Event::KeyType::PageUp));

		//Numpad0~9
		if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9)
			return static_cast<Event::KeyType>(wParam - VK_NUMPAD0 + static_cast<int>(Event::KeyType::Numpad0));

		switch (wParam)
		{
		case VK_SHIFT:
			return Event::KeyType::Shift;
		case VK_CONTROL:
			return Event::KeyType::LCtrl;
		case VK_MENU:
			return Event::KeyType::LAlt;
		case VK_SPACE:
			return Event::KeyType::Space;
		case VK_BACK:
			return Event::KeyType::BackSpace;
		case VK_RETURN:
			return Event::KeyType::Enter;
		case VK_TAB:
			return Event::KeyType::Tab;
		case VK_ESCAPE:
			return Event::KeyType::Esc;
		case VK_INSERT:
			return Event::KeyType::Insert;
		case VK_DELETE:
			return Event::KeyType::Delete;
		case VK_ADD:
			return Event::KeyType::NumpadAdd;
		case VK_SUBTRACT:
			return Event::KeyType::NumpadSubtract;
		case VK_DIVIDE:
			return Event::KeyType::NumpadDivide;
		case VK_MULTIPLY:
			return Event::KeyType::NumpadMultiply;
		case VK_DECIMAL:
			return Event::KeyType::NumpadDecimal;
		case VK_OEM_COMMA: //,
			return Event::KeyType::Comma;
		case VK_OEM_PERIOD: //.
			return Event::KeyType::Period;
		case VK_OEM_2:
			return Event::KeyType::Slash;
		case VK_OEM_5:
			return Event::KeyType::BackSlash;
		case VK_OEM_MINUS:
			return Event::KeyType::Minus;
		case VK_OEM_PLUS:
			return Event::KeyType::Equal;
		case VK_OEM_4:
			return Event::KeyType::LBracket;
		case VK_OEM_6:
			return Event::KeyType::RBracket;
		case VK_OEM_1:
			return Event::KeyType::Semicolon;
		case VK_OEM_7:
			return Event::KeyType::Colon;
		case VK_SNAPSHOT:
			return Event::KeyType::Print;
		case VK_SCROLL:
			return Event::KeyType::Scroll;
		case VK_PAUSE:
			return Event::KeyType::Pause;
		case VK_NUMLOCK:
			return Event::KeyType::NumLock;
		case VK_OEM_3:
			return Event::KeyType::Grave;
		}
		return Event::KeyType::Unknown;
	}

	void WindowImplWin32::SetTitle(std::string_view title)
	{
		std::wstring wtitle = sh::core::Util::U8StringToWstring(std::string{ title });
		SetWindowTextW(window, wtitle.c_str());
	}

	auto WindowImplWin32::GetWidth() const -> uint32_t
	{
		RECT rect;
		GetClientRect(window, &rect);
		return rect.right - rect.left;
	}
	auto WindowImplWin32::GetHeight() const -> uint32_t
	{
		RECT rect;
		GetClientRect(window, &rect);
		return rect.bottom - rect.top;
	}
}