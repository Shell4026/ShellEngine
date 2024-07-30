#include "Observer.h"

#include <iostream>
namespace sh::core
{
	bool Observer::Event::operator<(const Event& other) const
	{
		if(priority != other.priority)
			return priority < other.priority;
		return handle < other.handle;
	}
	Observer::Event::Event(Event&& other) noexcept :
		handle(other.handle), func(std::move(other.func)), priority(other.priority)
	{
	}

	Observer::Observer() :
		nextHandle(0)
	{
	}
	Observer::~Observer()
	{
	}

	auto Observer::RegisterEvent(const std::function<void()>& func, int priority) -> EventHandle
	{
		Event event;
		event.handle = nextHandle;
		event.func = func;
		event.priority = priority;
		
		auto it = events.insert(std::move(event));
		const Event* eventPtr = &(*it.first);
		eventMap.insert({ nextHandle, eventPtr });

		return nextHandle++;
	}

	bool Observer::RemoveEvent(EventHandle handle)
	{
		auto it = eventMap.find(handle);
		if (it == eventMap.end())
			return false;

		events.erase(*it->second);
		eventMap.erase(it);

		return true;
	}

	void Observer::Notify()
	{
		for (auto& event : events)
		{
			event.func();
		}
	}
}