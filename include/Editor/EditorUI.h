#pragma once

#include "Export.h"
#include "UI.h"
#include "ExplorerUI.h"
#include "Viewport.h"
#include "Hierarchy.h"
#include "Project.h"
#include "Inspector.h"

#include "Game/World.h"

#include <vector>
#include <mutex>

namespace sh::editor
{
	class EditorWorld;

	class EditorUI : public UI
	{
	private:
		EditorWorld& world;

		float hierarchyWidth;
		float hierarchyHeight;

		ImGuiID dockspaceId;

		ExplorerUI explorer;
		Viewport viewport;
		Hierarchy hierarchy;
		Project project;
		Inspector inspector;

		bool bOpenExplorer;
		bool bDirty;
	private:
		inline void SetDockNode();
		inline void DrawInspector();
	public:
		SH_EDITOR_API EditorUI(EditorWorld& world, game::ImGUImpl& imgui);
		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();
		
		SH_EDITOR_API auto GetViewport() -> Viewport&;

		SH_EDITOR_API void Clean();
	};
}