#pragma once

#include "Export.h"
#include "UI.h"
#include "ExplorerUI.h"

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
		float viewportWidthLast;
		float viewportHeightLast;

		int selected;

		ImGuiID dockspaceId;

		ExplorerUI explorer;

		std::array<VkDescriptorSet, 2> viewportDescSet; // 게임 스레드, 렌더 스레드

		bool bViewportDocking : 1;
		bool bHierarchyDocking : 1;
		bool bAddComponent : 1;
		bool bOpenExplorer : 1;
		bool bChangedViewportSize : 1;
	private:
		inline void SetDockNode();
		inline void DrawViewport();
		inline void DrawHierarchy();
		inline void DrawInspector();
		inline void DrawProject();
		inline void ChangedViewportSize();
	public:
		SH_EDITOR_API EditorUI(game::World& world, const game::ImGUI& imgui, std::mutex& renderMutex);
		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();
		SH_EDITOR_API void SyncRenderThread();
	};
}