#include "Input.h"

#include "Window/Event.h"

#include <cstdint>

namespace sh::game
{
	std::bitset<100> Input::keyPressing{};

	void Input::Update(window::Event event)
	{
		if (event.type == window::Event::EventType::KeyDown)
			keyPressing[static_cast<uint32_t>(event.keyType)] = true;
		else if(event.type == window::Event::EventType::KeyUp)
			keyPressing[static_cast<uint32_t>(event.keyType)] = false;
	}

	bool Input::GetKeyDown(KeyCode keycode)
	{
		return  keyPressing[static_cast<uint32_t>(keycode)];
	}
}