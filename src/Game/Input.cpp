#include "Input.h"

#include "Window/Event.h"

#include <cstdint>
#include <imgui.h>

namespace sh::game
{
	std::bitset<100> Input::keyPressing{};
	std::bitset<3> Input::mousePressing{};
	glm::vec2 Input::mousePos{};
	core::SyncArray<glm::vec2> Input::mouseDelta{};
	float Input::wheelDelta(0.f);

	const glm::vec2& Input::mousePosition(mousePos);
	const glm::vec2& Input::mousePositionDelta(mouseDelta[GAME_THREAD]);
	const float& Input::mouseWheelDelta(window::Event::MouseWheelScrolled::delta);

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
			wheelDelta = window::Event::MouseWheelScrolled::delta;
		}

		mouseDelta[RENDER_THREAD].x = mousePos.x;
		mouseDelta[RENDER_THREAD].y = mousePos.y;
		mousePos.x = window::Event::MousePosition::mouseX;
		mousePos.y = window::Event::MousePosition::mouseY;
		mouseDelta[RENDER_THREAD].x = mousePos.x - mouseDelta[RENDER_THREAD].x;
		mouseDelta[RENDER_THREAD].y = mousePos.y - mouseDelta[RENDER_THREAD].y;
	}

	bool Input::GetKeyDown(KeyCode keycode)
	{
		//if (ImGui::GetIO().WantCaptureKeyboard)
		//	return false;
		return  keyPressing[static_cast<uint32_t>(keycode)];
	}
	bool Input::GetMouseDown(MouseType mouseType)
	{
		return mousePressing[static_cast<uint32_t>(mouseType)];
	}
	void Input::SyncGameThread()
	{
		mouseDelta[GAME_THREAD] = mouseDelta[RENDER_THREAD];
	}
}