#pragma once

#include "Export.h"

namespace sh::window {
	class Event {
	public:
		enum class EventType
		{
			Close,
			Resize,
			Move,
			KeyDown,
			KeyUp,
			MousePressed,
			MouseReleased,
			MouseWheelScrolled,
			MouseMove,
			WindowFocus,
			WindowFocusOut,
			Unknown
		};

		enum class MouseType
		{
			Left,
			Right,
			Middle
		};

		enum class KeyType
		{
			Num0, Num1, Num2, Num3,Num4,
			Num5, Num6, Num7, Num8,Num9,
			F1, F2, F3, F4, F5, F6, F7,
			F8, F9, F10, F11, F12,
			A, B, C, D, E, F, G, H, I,
			J, K, L, M, N, O, P, Q, R,
			S, T, U, V, W, X, Y, Z,
			Shift, LCtrl, RCtrl,
			LAlt, RAlt,
			Space, BackSpace, Enter, Tab, Esc,
			Left, Up, Right, Down,
			Delete, Insert, PageUp, PageDown, End, Home,
			Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
			Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
			NumpadAdd, NumpadSubtract, NumpadDivide, NumpadMultiply, NumpadDecimal,
			Comma, Period, Slash, BackSlash, Minus, Equal,
			Grave, LBracket, RBracket, Colon, Semicolon,
			Print, Scroll, Pause,
			NumLock,
			Unknown
		};

		struct MouseWheelScrolled
		{
			SH_WINDOW_API static float delta;
		};

		struct MousePosition
		{
			SH_WINDOW_API static int mouseX;
			SH_WINDOW_API static int mouseY;
		};

		EventType type;
		MouseType mouseType;
		KeyType keyType;

	};
}//namespace