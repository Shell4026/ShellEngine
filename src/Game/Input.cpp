#include "Input.h"

#include "Window/Event.h"

#include <cstdint>
#include <imgui.h>

namespace sh::game
{
	std::bitset<100> Input::keyPressing{};
	std::bitset<3> Input::mousePressing{};
	glm::vec2 Input::mousePos{0.f, 0.f};
	glm::vec2& Input::mousePosition(mousePos);

	void Input::Update(window::Event event)
	{
		if (event.type == window::Event::EventType::KeyDown)
			keyPressing[static_cast<uint32_t>(event.keyType)] = true;
		else if(event.type == window::Event::EventType::KeyUp)
			keyPressing[static_cast<uint32_t>(event.keyType)] = false;

		if (event.type == sh::window::Event::EventType::MousePressed)
			mousePressing[static_cast<uint32_t>(event.mouseType)] = true;
		else if(event.type == sh::window::Event::EventType::MouseReleased)
			mousePressing[static_cast<uint32_t>(event.mouseType)] = false;

		if (event.type == sh::window::Event::EventType::MouseWheelScrolled)
		{
			std::cout << sh::window::Event::MouseWheelScrolled::delta << '\n';
		}

		mousePosition.x = window::Event::MousePosition::mouseX;
		mousePosition.y = window::Event::MousePosition::mouseY;
	}

	bool Input::GetKeyDown(KeyCode keycode)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return false;
		return  keyPressing[static_cast<uint32_t>(keycode)];
	}
	bool Input::GetMouseDown(MouseType mouseType)
	{
		return mousePressing[static_cast<uint32_t>(mouseType)];
	}
}