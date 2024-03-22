#pragma once

namespace sh {
	struct Event {
		enum class EventType
		{
			Close,
			Move,
			KeyDown,
			KeyUp,
			MousePressed,
			MouseReleased,
			Unknown
		};

		enum class MouseType
		{
			Left,
			Right,
			Middle
		};

		EventType type;
		MouseType mouseType;
	};
}