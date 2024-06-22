#pragma once

#include "Export.h"
#include "UI.h"
#include "ExplorerUI.h"

#include "Game/World.h"

#include <vector>
namespace sh::editor
{
	class EditorUI : public UI
	{
	private:
		game::World& world;

		float hierarchyWidth;
		float hierarchyHeight;

		int selected;

		ImGuiID dockspaceId;

		ExplorerUI explorer;

		bool bViewportDocking : 1;
		bool bHierarchyDocking : 1;
		bool bAddComponent : 1;
		bool bOpenExplorer : 1;
	private:
		inline void SetDockNode();
		inline void DrawViewport();
		inline void DrawHierarchy();
		inline void DrawInspector();
		inline void DrawProject();
	public:
		SH_EDITOR_API EditorUI(game::World& world, const game::ImGUI& imgui);
		SH_EDITOR_API void Update();
	};
}