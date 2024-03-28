#include "Unix/WindowImplUnix.h"

#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <iostream>

namespace sh {
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
		return win;
	}

	void WindowImplUnix::Close()
	{
		XDestroyWindow(display, win);
		XCloseDisplay(display);
	}

	void WindowImplUnix::ProcessEvent()
	{
		XNextEvent(display, &e);
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
		}
	}

	auto WindowImplUnix::CovertKeyCode(unsigned int keycode) -> Event::KeyType
	{
		std::cout << keycode << '\n';
		constexpr Event::KeyType alphabet[] = { 
			Event::KeyType::Q, Event::KeyType::W, Event::KeyType::E, Event::KeyType::R,
			Event::KeyType::T, Event::KeyType::Y, Event::KeyType::U, Event::KeyType::I,
			Event::KeyType::O, Event::KeyType::P, Event::KeyType::A, Event::KeyType::S,
			Event::KeyType::D, Event::KeyType::F, Event::KeyType::G, Event::KeyType::H,
			Event::KeyType::J, Event::KeyType::K, Event::KeyType::L, Event::KeyType::Z,
			Event::KeyType::X, Event::KeyType::C, Event::KeyType::V, Event::KeyType::B,
			Event::KeyType::N, Event::KeyType::M
		};
		switch (keycode)
		{
		case 19:
			return Event::KeyType::Num0;
		//C++ 표준 문법은 아님!
		case 10 ... 18:
			return static_cast<Event::KeyType>(keycode - 10 + static_cast<int>(Event::KeyType::Num1));
		case 67 ... 76:
			return static_cast<Event::KeyType>(keycode - 67 + static_cast<int>(Event::KeyType::F1));
		case 95 ... 96:
			return static_cast<Event::KeyType>(keycode - 95 + static_cast<int>(Event::KeyType::F11));
		case 24 ... 33: //QWERTYUIOP
			return static_cast<Event::KeyType>(alphabet[keycode - 24]);
		case 38 ... 46: //ASDEFGHJKL
			return static_cast<Event::KeyType>(alphabet[keycode - 38 + 10]);
		case 52 ... 58: //ZXCVBNM
			return static_cast<Event::KeyType>(alphabet[keycode - 52 + 19]);
		case 50:
			return Event::KeyType::LShift;
		case 62:
			return Event::KeyType::RShift;
		case 37:
			return Event::KeyType::LCtrl;
		case 109:
			return Event::KeyType::RCtrl;
		case 64:
			return Event::KeyType::LAlt;
		//case 113:
			//return Event::KeyType::RAlt;
		case 65:
			return Event::KeyType::Space;
		case 22:
			return Event::KeyType::BackSpace;
		case 36:
			return Event::KeyType::Enter;
		case 23:
			return Event::KeyType::Tab;
		case 9:
			return Event::KeyType::Esc;
		case 113:
			return Event::KeyType::Left;
		case 111:
			return Event::KeyType::Up;
		case 114:
			return Event::KeyType::Right;
		case 116:
			return Event::KeyType::Down;
		case 119:
			return Event::KeyType::Delete;
		case 118:
			return Event::KeyType::Insert;
		case 112:
			return Event::KeyType::PageUp;
		case 117:
			return Event::KeyType::PageDown;
		case 115:
			return Event::KeyType::End;
		case 110:
			return Event::KeyType::Home;
		}
		return Event::KeyType::Unknown;
	}
}//namespace