#include "Project.h"

#include "Game/World.h"

namespace sh::editor
{
	Project::Project(game::ImGUImpl& imgui, game::World& world) :
		UI(imgui), world(world),
		currentPath(std::filesystem::current_path().string()),
		folderIcon(world.renderer)
	{
		for (const auto& entry : std::filesystem::directory_iterator(currentPath)) 
		{
			if (std::filesystem::is_directory(entry.path()))
				dirs.push_back(entry.path().filename());
		}

		auto folderTex = world.textures.GetResource("FolderIcon");
		assert(folderTex);
		render::ITextureBuffer* texBuffer = folderTex->GetBuffer();

		if (world.renderer.apiType == render::RenderAPI::Vulkan)
		{
			folderIcon.Create(*folderTex);
		}
	}

	SH_EDITOR_API void Project::Update()
	{

	}
	SH_EDITOR_API void Project::Render()
	{
		static ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Project", nullptr, style);

		float iconSize = 50.0f;
		float availableWidth = ImGui::GetContentRegionAvail().x;
		float cursorX = ImGui::GetCursorPosX();
		float spacing = ImGui::GetStyle().ItemSpacing.x;

		for (auto& dir : dirs)
		{
			ImGui::BeginGroup();
			if (cursorX + iconSize > availableWidth)
			{
				ImGui::NewLine();
				cursorX = ImGui::GetCursorPosX();
			}
			if (ImGui::ImageButton(folderIcon, ImVec2{ iconSize, iconSize }))
			{

			}
			std::string folderName = dir.filename().string();
			float textWidth = ImGui::CalcTextSize(folderName.c_str()).x;
			float textOffset = (iconSize - textWidth);
			if (textOffset > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
			ImGui::TextWrapped("%s", folderName.c_str());
			ImGui::EndGroup();

			ImGui::SameLine(0.0f, spacing);
			cursorX += iconSize + spacing;
		}
		
		ImGui::End();
	}
}