#pragma once

#include "Export.h"
#include "Core/ISyncable.h"

#include "Window/Window.h"

#include "glm/vec2.hpp"
#include <bitset>
namespace sh::game
{
	class Input
	{
	public:
		using KeyCode = window::Event::KeyType;
		using MouseType = window::Event::MouseType;
	public:
		SH_GAME_API static void Update();
		/// @brief 이벤트를 받아서 상태를 업데이트 하는 함수.
		/// @param event 윈도우 이벤트 객체
		SH_GAME_API static void UpdateEvent(window::Event event);

		/// @brief 현재 키가 눌러져 있는지 확인하는 함수.
		/// @param keycode 키코드 enum
		/// @param bIgnoreGui UI 입력을 무시할 것인지
		/// @return 눌렀으면 true, 아니면 false.
		SH_GAME_API static bool GetKeyDown(KeyCode keycode, bool bIgnoreGui = false);
		/// @brief 현재 프레임에서 키를 눌렀는지 확인하는 함수.
		/// @param keycode 키코드 enum
		/// @param bIgnoreGui UI 입력을 무시할 것인지
		/// @return 눌렀으면 true, 아니면 false.
		SH_GAME_API static auto GetKeyPressed(KeyCode keycode, bool bIgnoreGui = false) -> bool;
		/// @brief 현재 프레임에서 키를 뗐는지 확인하는 함수.
		/// @param keycode 키코드 enum
		/// @param bIgnoreGui UI 입력을 무시할 것인지
		/// @return 뗐으면 true, 아니면 false.
		SH_GAME_API static auto GetKeyReleased(KeyCode keycode, bool bIgnoreGui = false) -> bool;
		/// @brief 현재 마우스가 눌러져 있는지 확인하는 함수.
		/// @param mouseType 마우스 타입 enum
		/// @return 눌렀으면 true, 아니면 false
		SH_GAME_API static bool GetMouseDown(MouseType mouseType);
		/// @brief 현재 프레임에서 마우스를 눌렀는지 확인하는 함수.
		/// @param MouseType 마우스 타입 enum
		/// @return 눌렀으면 true, 아니면 false.
		SH_GAME_API static bool GetMousePressed(MouseType mouseType);
		/// @brief 현재 프레임에서 마우스를 뗐는지 확인하는 함수.
		/// @param MouseType 마우스 타입 enum
		/// @return 뗏으면 true, 아니면 false.
		SH_GAME_API static bool GetMouseReleased(MouseType mouseType);
		SH_GAME_API static void ResetKeyState();
	public:
		SH_GAME_API static const glm::vec2& mousePosition;
		SH_GAME_API static const glm::vec2& mousePositionDelta;
		SH_GAME_API static const float& mouseWheelDelta;
	private:
		static std::bitset<100> keyPressing;
		static std::bitset<100> keyPressingOneFrame;
		static std::bitset<100> keyReleasingOneFrame;
		static std::bitset<3> mousePressing;
		static std::bitset<3> mousePressingOneFrame;
		static std::bitset<3> mouseReleasingOneFrame;
		static glm::vec2 mousePos;
		static glm::vec2 mouseDelta;
		static float wheelDelta;
	};
}