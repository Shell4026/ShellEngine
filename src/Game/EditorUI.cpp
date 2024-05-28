#include "EditorUI.h"

namespace sh::game
{
	EditorUI::EditorUI(World& world) :
		world(world)
	{
	}

	void EditorUI::DrawHierarchy()
	{
		ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoResize;
		
		ImGui::SetNextWindowPos(ImVec2{ 0, 20 });
		ImGui::SetNextWindowSize(ImVec2{150, 768});
		ImGui::Begin("Hierarchy", nullptr, style);
		ImGui::End();
	}

	void EditorUI::Update()
	{
		if (!ImGUI::IsInit())
			return;

		DrawHierarchy();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{

				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}