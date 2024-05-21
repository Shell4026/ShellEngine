#pragma once

#include "Export.h"

#include "Window/Window.h"

#include <bitset>
namespace sh::game
{
	class Input
	{
	private:
		static std::bitset<100> keyPressing;
	public:
		using KeyCode = window::Event::KeyType;
	public:
		SH_GAME_API static void Update(window::Event event);

		SH_GAME_API static bool GetKeyDown(KeyCode keycode);
	};
}