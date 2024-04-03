#include "WindowImpl.h"

namespace sh::window {
	WindowImpl::~WindowImpl()
	{
	}

	void WindowImpl::PushEvent(const Event& e)
	{
		events.push(e);
	}

	Event WindowImpl::PopEvent()
	{
		Event e = events.front();
		events.pop();
		return e;
	}

	bool WindowImpl::IsEmptyEvent() const
	{
		return events.empty();
	}
}