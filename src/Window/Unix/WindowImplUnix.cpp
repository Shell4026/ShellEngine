#include "Unix/WindowImplUnix.h"

#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <iostream>

namespace sh::window {
	WindowImplUnix::~WindowImplUnix()
	{
		Close();
	}

	auto WindowImplUnix::Create(const std::string& title, int wsize, int hsize) -> WinHandle
	{
		std::cout << "WindowImplUnix::Create()\n";

		//디스플레이 서버에 접속한다.
		display = XOpenDisplay(NULL);

		screen = DefaultScreen(display);

		XSetWindowAttributes attrs;
		attrs.background_pixel = WhitePixel(display, screen);
		attrs.event_mask = FocusChangeMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask;

		root = XDefaultRootWindow(display);
		win = XCreateWindow(display, root, 0, 0, wsize, hsize, 1,
			CopyFromParent, InputOutput, CopyFromParent,
			CWBackPixel | CWEventMask, &attrs);

		//창 제목 설정
		Atom netWmName = XInternAtom(display, "_NET_WM_NAME", false);
		Atom utf8String = XInternAtom(display, "UTF8_STRING", false);
		XChangeProperty(display, win, netWmName, utf8String, 8, PropModeReplace,
			(const unsigned char*)title.c_str(), 4);
		
		//event_mask에 해당하는 이벤트를 선택
		XSelectInput(display, win, attrs.event_mask); 

		//창 닫는 이벤트 등록
		wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", false); 
		XSetWMProtocols(display, win, &wmDeleteMessage, 1);

		XMapWindow(display, win);
		XFlush(display);

		return std::make_pair(display, win);
	}

	void WindowImplUnix::Close()
	{
		XDestroyWindow(display, win);
		XCloseDisplay(display);
	}

	//현재 윈도우에서 발생한 이벤트인가?
	auto WindowImplUnix::CheckEvent(Display*, XEvent* evt, XPointer userData) -> int {
		return evt->xany.window == reinterpret_cast<Window>(userData);
	}

	void WindowImplUnix::ProcessEvent()
	{
		while (XCheckIfEvent(display, &e, &CheckEvent, reinterpret_cast<XPointer>(win)))
		{
			switch (e.type)
			{
				Event evt;
				//window
			case ClientMessage:
				if (e.xclient.data.l[0] == wmDeleteMessage)
				{
					evt.type = Event::EventType::Close;
					PushEvent(evt);
				}
				break;
			case FocusIn:
				evt.type = Event::EventType::WindowFocus;
				PushEvent(evt);
				break;
			case FocusOut:
				evt.type = Event::EventType::WindowFocusOut;
				PushEvent(evt);
				break;

				//Keyboard
			case KeyPress:
			{
				XKeyEvent* keyEvent = reinterpret_cast<XKeyEvent*>(&e);
				evt.type = Event::EventType::KeyDown;
				evt.keyType = CovertKeyCode(keyEvent->keycode);
				PushEvent(evt);
				break;
			}
			//Mouse
			case ButtonPress:
			{
				XButtonEvent* buttonEvent = reinterpret_cast<XButtonEvent*>(&e);

				switch (buttonEvent->button)
				{
				case Button1:
					evt.type = Event::EventType::MousePressed;
					evt.mouseType = Event::MouseType::Left;
					PushEvent(evt);
					break;
				case Button2:
					evt.type = Event::EventType::MousePressed;
					evt.mouseType = Event::MouseType::Middle;
					PushEvent(evt);
					break;
				case Button3:
					evt.type = Event::EventType::MousePressed;
					evt.mouseType = Event::MouseType::Right;
					PushEvent(evt);
					break;
				case Button4:
				case Button5:
					evt.type = Event::EventType::MouseWheelScrolled;
					Event::MouseWheelScrolled::delta = (buttonEvent->button == Button4) ? 1.0f : -1.0f;
					PushEvent(evt);
					break;
				}
				break;
			}
			}//switch
		}

	}

	auto WindowImplUnix::GetDisplay() -> Display*
	{
		return display;
	}

	auto WindowImplUnix::CovertKeyCode(unsigned int keycode) -> Event::KeyType
	{
		KeySym keySym = XkbKeycodeToKeysym(display, keycode, 0, 0);
		std::cout << keySym << '\n';
		switch (keySym)
		{
		//A...B : C++ 표준 문법은 아님!
		case XK_0 ... XK_9:
			return static_cast<Event::KeyType>(keySym - XK_0 + static_cast<int>(Event::KeyType::Num0));
		case XK_F1 ... XK_F12: 
			return static_cast<Event::KeyType>(keySym - XK_F1 + static_cast<int>(Event::KeyType::F1));
		case XK_a ... XK_z:
			return static_cast<Event::KeyType>(keySym - XK_a + static_cast<int>(Event::KeyType::A));
		case XK_A ... XK_Z:
			return static_cast<Event::KeyType>(keySym - XK_A + static_cast<int>(Event::KeyType::A));
		case XK_Shift_L: return Event::KeyType::Shift;
		case XK_Shift_R: return Event::KeyType::Shift;
		case XK_Control_L: return Event::KeyType::LCtrl;
		case XK_Control_R: return Event::KeyType::RCtrl;
		case XK_Alt_L: return Event::KeyType::LAlt;
		case XK_Alt_R: return Event::KeyType::RAlt;
		case XK_space: return Event::KeyType::Space;
		case XK_BackSpace: return Event::KeyType::BackSpace;
		case XK_Return: return Event::KeyType::Enter;
		case XK_Tab: return Event::KeyType::Tab;
		case XK_Escape: return Event::KeyType::Esc;
		case XK_Left: return Event::KeyType::Left;
		case XK_Up: return Event::KeyType::Up;
		case XK_Right: return Event::KeyType::Right;
		case XK_Down: return Event::KeyType::Down;
		case XK_Delete : return Event::KeyType::Delete;
		case XK_Insert: return Event::KeyType::Insert;
		case XK_Page_Up: return Event::KeyType::PageUp;
		case XK_Page_Down: return Event::KeyType::PageDown;
		case XK_End: return Event::KeyType::End;
		case XK_Home: return Event::KeyType::Home;
		case XK_KP_0 ... XK_KP_9: 
			return static_cast<Event::KeyType>(keySym - XK_KP_0 + static_cast<int>(Event::KeyType::Numpad0));
		case XK_KP_Add: return Event::KeyType::NumpadAdd;
		case XK_KP_Subtract: return Event::KeyType::NumpadSubtract;
		case XK_KP_Divide: return Event::KeyType::NumpadDivide;
		case XK_KP_Multiply: return Event::KeyType::NumpadMultiply;
		case XK_KP_Decimal: return Event::KeyType::NumpadDecimal;
		case XK_comma: return Event::KeyType::Comma;
		case XK_period: return Event::KeyType::Period;
		case XK_slash: return Event::KeyType::Slash;
		case XK_minus: return Event::KeyType::Minus;
		case XK_equal: return Event::KeyType::Equal;
		case XK_grave: return Event::KeyType::Grave;
		case XK_bracketleft: return Event::KeyType::LBracket;
		case XK_braceright: return Event::KeyType::RBracket;
		case XK_colon: return Event::KeyType::Colon;
		case XK_semicolon: return Event::KeyType::Semicolon;
		case XK_Print: return Event::KeyType::Print;
		case XK_Scroll_Lock: return Event::KeyType::Scroll;
		case XK_Pause: return Event::KeyType::Pause;
		case XK_Num_Lock : return Event::KeyType::NumLock;
		}
		return Event::KeyType::Unknown;
	}
}//namespace