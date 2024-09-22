#pragma once

#include "Export.h"

#include "Window/Window.h"

#include "glm/vec2.hpp"
#include <bitset>
namespace sh::game
{
	class Input
	{
	private:
		static std::bitset<100> keyPressing;
		static std::bitset<3> mousePressing;
		static glm::vec2 mousePos;
	public:
		using KeyCode = window::Event::KeyType;
		using MouseType = window::Event::MouseType;
		SH_GAME_API static glm::vec2& mousePosition;
	public:
		SH_GAME_API static void Update(window::Event event);

		/// @brief 현재 키가 눌러져 있는지 확인하는 함수.
		/// @param keycode 키코드 enum
		/// @return 눌렀으면 true, 아니면 false.
		SH_GAME_API static bool GetKeyDown(KeyCode keycode);
		/// @brief 현재 마우스가 눌러져 있는지 확인하는 함수.
		/// @param mouseType 마우스 타입 enum
		/// @return 눌렀으면 true, 아니면 false
		SH_GAME_API static bool GetMouseDown(MouseType mouseType);
	};
}