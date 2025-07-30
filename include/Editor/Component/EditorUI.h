#pragma once
#include "Export.h"
#include "../UI/ExplorerUI.h"
#include "../UI/Viewport.h"
#include "../UI/Hierarchy.h"
#include "../UI/Project.h"
#include "../UI/Inspector.h"

#include "Game/Component/UI.h"
#include "Game/World.h"

#include <vector>
#include <mutex>

namespace sh::editor
{
	class EditorUI : public game::UI
	{
		COMPONENT(EditorUI, "Editor")	
	private:
		float hierarchyWidth;
		float hierarchyHeight;

		ImGuiID dockspaceId;

		Project* project = nullptr;
		std::unique_ptr<ExplorerUI> explorer;
		std::unique_ptr<Viewport> viewport;
		std::unique_ptr<Hierarchy> hierarchy;
		std::unique_ptr<Inspector> inspector;

		bool bDirty;
		bool bPlaying = false;
	private:
		inline void SetDockNode();
		inline void DrawMenu();
	public:
		SH_EDITOR_API EditorUI(game::GameObject& owner);

		SH_EDITOR_API void Awake() override;

		SH_EDITOR_API void BeginUpdate() override;
		SH_EDITOR_API void Update() override;
		
		SH_EDITOR_API auto GetViewport() -> Viewport&;
		SH_EDITOR_API auto GetHierarchy() -> Hierarchy&;

		SH_EDITOR_API void Clean();

		SH_EDITOR_API void SetProject(Project& project);
	};
}