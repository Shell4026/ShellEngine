#include "Input.h"

#include "Window/Event.h"

#include <cstdint>
#include <imgui.h>

namespace sh::game
{
	std::bitset<100> Input::keyPressing{};
	std::bitset<100> Input::keyPressingOneFrame{};
	std::bitset<100> Input::keyReleasingOneFrame{};
	std::bitset<3> Input::mousePressing{};
	std::bitset<3> Input::mousePressingOneFrame;
	std::bitset<3> Input::mouseReleasingOneFrame;
	glm::vec2 Input::mousePos{};
	glm::vec2 Input::mouseDelta{};
	float Input::wheelDelta{ 0.f };

	const glm::vec2& Input::mousePosition(mousePos);
	const glm::vec2& Input::mousePositionDelta(mouseDelta);
	const float& Input::mouseWheelDelta(wheelDelta);

	SH_GAME_API void Input::Update()
	{
		wheelDelta = 0.f;

		mouseDelta = { 0.f, 0.f };

		keyPressingOneFrame.reset();
		keyReleasingOneFrame.reset();

		mousePressingOneFrame.reset();
		mouseReleasingOneFrame.reset();
	}

	SH_GAME_API void Input::UpdateEvent(window::Event event)
	{
		if (event.type == window::Event::EventType::KeyDown)
		{
			const auto keyType = static_cast<uint32_t>(std::get<2>(event.data).keyType);
			if (!keyPressing[keyType])
				keyPressingOneFrame[keyType] = true;
			keyPressing[keyType] = true;
		}
		else if(event.type == window::Event::EventType::KeyUp)
		{
			const auto keyType = static_cast<uint32_t>(std::get<2>(event.data).keyType);
			keyPressing[keyType] = false;
			keyReleasingOneFrame[keyType] = true;
		}

		if (event.type == sh::window::Event::EventType::MousePressed)
		{
			const auto mouseType = static_cast<uint32_t>(std::get<1>(event.data));
			mousePressing[mouseType] = true;
			mousePressingOneFrame[mouseType] = true;
		}
		else if(event.type == sh::window::Event::EventType::MouseReleased)
		{
			const auto mouseType = static_cast<uint32_t>(std::get<1>(event.data));
			mousePressing[mouseType] = false;
			mouseReleasingOneFrame[mouseType] = true;
		}

		if (event.type == window::Event::EventType::MouseWheelScrolled)
			wheelDelta = window::Event::MouseWheelScrolled::delta;

		mouseDelta.x = mousePos.x;
		mouseDelta.y = mousePos.y;
		mousePos.x = window::Event::MousePosition::mouseX;
		mousePos.y = window::Event::MousePosition::mouseY;
		mouseDelta.x = mousePos.x - mouseDelta.x;
		mouseDelta.y = mousePos.y - mouseDelta.y;
	}

	SH_GAME_API bool Input::GetKeyDown(KeyCode keycode, bool bIgnoreGui)
	{
		if (!bIgnoreGui && ImGui::GetIO().WantTextInput)
			return false;
		return  keyPressing[static_cast<uint32_t>(keycode)];
	}
	SH_GAME_API auto Input::GetKeyPressed(KeyCode keycode, bool bIgnoreGui) -> bool
	{
		if (!bIgnoreGui && ImGui::GetIO().WantTextInput)
			return false;
		return  keyPressingOneFrame[static_cast<uint32_t>(keycode)];
	}
	SH_GAME_API auto Input::GetKeyReleased(KeyCode keycode, bool bIgnoreGui) -> bool
	{
		if (!bIgnoreGui && ImGui::GetIO().WantTextInput)
			return false;
		return  keyReleasingOneFrame[static_cast<uint32_t>(keycode)];
	}
	SH_GAME_API bool Input::GetMouseDown(MouseType mouseType)
	{
		return mousePressing[static_cast<uint32_t>(mouseType)];
	}
	SH_GAME_API bool Input::GetMousePressed(MouseType mouseType)
	{
		return mousePressingOneFrame[static_cast<uint32_t>(mouseType)];
	}
	SH_GAME_API bool Input::GetMouseReleased(MouseType mouseType)
	{
		return mouseReleasingOneFrame[static_cast<uint32_t>(mouseType)];
	}
	SH_GAME_API void Input::ResetKeyState()
	{
		keyReleasingOneFrame = keyPressing;
		keyPressing.reset();
	}
}