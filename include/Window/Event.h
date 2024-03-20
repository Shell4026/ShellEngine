#pragma once

namespace sh {
	struct Event {
		enum class EventType
		{
			Close
		};

		EventType type;
	};
}