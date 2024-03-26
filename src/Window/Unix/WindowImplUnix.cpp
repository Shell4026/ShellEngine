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

		XStoreName(display, win, title.c_str()); //창 제목 설정
		Xutf8SetWMProperties(display, win, title.c_str(), NULL, NULL, 0, NULL, NULL, NULL);

		XSelectInput(display, win, attrs.event_mask); //event_mask에 해당하는 이벤트를 선택
		//창 닫는 이벤트 등록
		wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False); 
		XSetWMProtocols(display, win, &wmDeleteMessage, 1);

		XMapWindow(display, win);
		XFlush(display);
		return 0;
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

		//mouse
		case ButtonPress:
			XButtonEvent* buttonEvent = (XButtonEvent*)&e;

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
				Event::MouseWheelScrolled::delta = (e.type == Button4) ? 1.0f : -1.0f;
				PushEvent(evt);
				break;
			}
			break;
		}
	}
}