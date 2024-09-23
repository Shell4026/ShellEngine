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
	private:
		static std::bitset<100> keyPressing;
		static std::bitset<3> mousePressing;
		static glm::vec2 mousePos;
		static core::SyncArray<glm::vec2> mouseDelta;
		static float wheelDelta;

		constexpr static int GAME_THREAD = core::ISyncable::GAME_THREAD;
		constexpr static int RENDER_THREAD = core::ISyncable::RENDER_THREAD;
	public:
		using KeyCode = window::Event::KeyType;
		using MouseType = window::Event::MouseType;
		SH_GAME_API static const glm::vec2& mousePosition;
		SH_GAME_API static const glm::vec2& mousePositionDelta;
		SH_GAME_API static const float& mouseWheelDelta;
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
		/// @brief 게임 스레드와 동기화 하는 함수.
		SH_GAME_API static void SyncGameThread();
	};
}