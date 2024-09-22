#pragma once

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
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;

		game::World& world;
		std::mutex* renderMutex;

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
	public:
		SH_EDITOR_API EditorUI(game::World& world, game::ImGUI& imgui, std::mutex& renderMutex);
		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();
		SH_EDITOR_API void SyncRenderThread();

		SH_EDITOR_API auto GetViewport() -> Viewport&;

		SH_EDITOR_API void Clean();
	};
}