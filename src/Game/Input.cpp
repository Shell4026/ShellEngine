#include "Input.h"

#include "Window/Event.h"

#include <cstdint>

namespace sh::game
{
	std::bitset<100> Input::keyPressing{};
	glm::vec2 Input::mousePos{0.f, 0.f};
	glm::vec2& Input::mousePosition(mousePos);

	void Input::Update(window::Event event)
	{
		if (event.type == window::Event::EventType::KeyDown)
			keyPressing[static_cast<uint32_t>(event.keyType)] = true;
		else if(event.type == window::Event::EventType::KeyUp)
			keyPressing[static_cast<uint32_t>(event.keyType)] = false;

		mousePosition.x = window::Event::MousePosition::mouseX;
		mousePosition.y = window::Event::MousePosition::mouseY;
	}

	bool Input::GetKeyDown(KeyCode keycode)
	{
		return  keyPressing[static_cast<uint32_t>(keycode)];
	}
}