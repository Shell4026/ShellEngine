#pragma once

#include "Export.h"
#include "ImGUI.h"

#include "Game/World.h"

namespace sh::game
{
	class EditorUI
	{
	private:
		World& world;
	private:
		void DrawHierarchy();
	public:
		SH_GAME_API EditorUI(World& world);
		SH_GAME_API void Update();
	};
}