#pragma once

#include "Export.h"

#include "SContainer.hpp"

#include <string_view>
#include <functional>
#include <map>
#include <set>
#include <memory>
#include <stdint.h>

namespace sh::core
{
	class Observer
	{
	private:
		struct Event
		{
			uint32_t handle;
			std::function<void()> func;
			int priority;

			Event() = default;
			Event(const Event& other) = default;
			Event(Event&& other) noexcept;

			bool operator<(const Event& other) const;
		};
		core::SSet<Event> events;
		core::SMap<int, const Event*> eventMap;

		uint32_t nextHandle;
	public:
		using EventHandle = int;
	public:
		SH_CORE_API Observer();
		SH_CORE_API virtual ~Observer();

		SH_CORE_API auto RegisterEvent(const std::function<void()>& func, int priority = 0) -> EventHandle;
		SH_CORE_API bool RemoveEvent(EventHandle handle);

		SH_CORE_API void Notify();
	};
}