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

		struct SelectedObj
		{
			GameObject* obj = nullptr;
			bool select = false;
		} selectedObj;
	private:
		void DrawViewport();
		void DrawHierarchy();
	public:
		SH_GAME_API EditorUI(World& world);
		SH_GAME_API void Update();
	};
}