#pragma once

namespace sh {
	struct Event {
		enum class EventType
		{
			Close,
			Move
		};

		EventType type;
	};
}