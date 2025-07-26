#include "UI/AssetExplorer.h"
#include "EditorWorld.h"

#include "Game/ImGUImpl.h"

#include "Window/Window.h"

#include <fmt/core.h>
namespace sh::editor
{
	AssetExplorer::AssetExplorer(const EditorWorld& world) :
		world(world)
	{
	}
	SH_EDITOR_API void AssetExplorer::SetAssetType(AssetType type)
	{
		this->type = type;
	}
	SH_EDITOR_API void AssetExplorer::Update()
	{
	}
	SH_EDITOR_API void AssetExplorer::Render()
	{
		float width = world.renderer.GetWindow().width / 2;
		float height = world.renderer.GetWindow().height / 2;
		ImGui::SetNextWindowSize(ImVec2{ width, height }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::Begin(fmt::format("AssetExplorer{}", static_cast<int>(type)).c_str());

		ImGui::Text("Hello");

		ImGui::End();
	}
}//namespace