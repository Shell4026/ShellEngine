#pragma once
#include "Editor/Export.h"
#include "../UI/ExplorerUI.h"
#include "../UI/Viewport.h"
#include "../UI/Hierarchy.h"
#include "../UI/Inspector.h"
#include "../UI/BundleViewer.h"
#include "../UI/FontGeneratorUI.h"

#include "Game/Component/UI.h"
#include "Game/World.h"

#include <vector>
#include <mutex>

namespace sh::editor
{
	class Project;
	class EditorUI : public game::UI
	{
		COMPONENT(EditorUI, "Editor")	
	public:
		SH_EDITOR_API EditorUI(game::GameObject& owner);

		SH_EDITOR_API void Awake() override;

		SH_EDITOR_API void BeginUpdate() override;
		SH_EDITOR_API void Update() override;
		
		SH_EDITOR_API auto GetViewport() -> Viewport&;
		SH_EDITOR_API auto GetHierarchy() -> Hierarchy&;

		SH_EDITOR_API void Clean();

		SH_EDITOR_API void SetProject(Project& project);
	private:
		void SetDockNode();
		void DrawMenu();
	private:
		float hierarchyWidth;
		float hierarchyHeight;

		ImGuiID dockspaceId;

		Project* project = nullptr;
		std::unique_ptr<ExplorerUI> explorer;
		std::unique_ptr<Viewport> viewport;
		std::unique_ptr<Hierarchy> hierarchy;
		std::unique_ptr<Inspector> inspector;
		std::unique_ptr<BundleViewer> bundleViewer;
		std::unique_ptr<FontGeneratorUI> fontGeneratorUI;

		bool bDirty;
	};
}