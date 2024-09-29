﻿#pragma once

#include "Export.h"
#include "UI.h"
#include "ExplorerUI.h"
#include "Viewport.h"

#include "Game/World.h"

#include <vector>
#include <mutex>

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
		Viewport viewport;

		bool bViewportDocking : 1;
		bool bHierarchyDocking : 1;
		bool bAddComponent : 1;
		bool bOpenExplorer : 1;
		bool bDirty : 1;
	private:
		inline void SetDockNode();
		inline void DrawHierarchy();
		inline void DrawInspector();
		inline void DrawProject();
		inline void Render();
	public:
		SH_EDITOR_API EditorUI(game::World& world, game::ImGUImpl& imgui);
		SH_EDITOR_API void Update();
		
		SH_EDITOR_API auto GetViewport() -> Viewport&;

		SH_EDITOR_API void Clean();
	};
}