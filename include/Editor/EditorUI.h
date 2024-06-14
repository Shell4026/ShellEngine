#pragma once

#include "Export.h"

#include "Game/ImGUI.h"
#include "Game/World.h"

#include <vector>
namespace sh::editor
{
	class EditorUI
	{
	private:
		game::World& world;
		const game::ImGUI& imgui;

		float hierarchyWidth;
		float hierarchyHeight;

		int selected;

		bool bAddComponent : 1;
	private:
		inline void DrawViewport();
		inline void DrawHierarchy();
		inline void DrawInspector();
		inline void DrawProject();
	public:
		SH_EDITOR_API EditorUI(game::World& world, const game::ImGUI& imgui);
		SH_EDITOR_API void Update();
	};
}