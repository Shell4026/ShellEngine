#include "Game/PCH.h"
#include "Project.h"
#include "EditorWorld.h"

#include "Game/TextureLoader.h"

namespace sh::editor
{
	bool Project::bInitResource = false;

	Project::Project(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui), world(world),
		rootPath(std::filesystem::current_path()),
		currentPath(rootPath),
		folderIcon(world.renderer), fileIcon(world.renderer)
	{
		InitResources();

		GetAllFiles(currentPath);
	}

	void Project::InitResources()
	{
		if (bInitResource)
			return;
		bInitResource = true;

		game::TextureLoader texLoader{ world.renderer };

		auto folderTex = world.textures.AddResource("FolderIcon", texLoader.Load("textures/folder.png"));
		folderIcon.Create(*folderTex);

		auto fileTex = world.textures.AddResource("FileIcon", texLoader.Load("textures/file.png"));
		fileIcon.Create(*fileTex);
	}

	void Project::GetAllFiles(const std::filesystem::path& path)
	{
		filesPath.clear();
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (rootPath == path)
			{
				if (std::filesystem::is_directory(entry.path()))
					filesPath.push_back(entry.path());
			}
			else
			{
				filesPath.push_back(entry.path());
			}
		}
	}
	auto Project::GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string
	{
		std::string result = path.filename().string();
		float currentSize = ImGui::CalcTextSize(result.c_str()).x;
		while (currentSize > maxSize)
		{
			result.pop_back();
			currentSize = ImGui::CalcTextSize(result.c_str()).x;
		}
		return result;
	}
	void Project::RenderParentFolder()
	{
		ImGui::BeginGroup();
		ImGui::ImageButton("../", folderIcon, ImVec2{ iconSize, iconSize }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, iconBackgroundColor);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			currentPath = currentPath.parent_path();
			GetAllFiles(currentPath);
		}
		float textWidth = ImGui::CalcTextSize("../").x;
		float textOffset = (iconSize - textWidth) / 2.f;
		if (textOffset >= 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::Text("../");
		ImGui::EndGroup();
	}

	SH_EDITOR_API void Project::Update()
	{
	}
	SH_EDITOR_API void Project::Render()
	{
		static ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Project", nullptr, style);

		float availableWidth = ImGui::GetContentRegionAvail().x;
		float cursorX = ImGui::GetCursorPosX();
		float spacing = ImGui::GetStyle().ItemSpacing.x;

		if (currentPath != rootPath)
		{
			RenderParentFolder();
			ImGui::SameLine();
		}
		for (auto& path : filesPath)
		{
			cursorX = ImGui::GetCursorPosX();
			if (cursorX + iconSize > availableWidth)
			{
				ImGui::NewLine();
				cursorX = ImGui::GetCursorPosX();
			}
			ImGui::BeginGroup();
			game::GUITexture* icon = &folderIcon;
			if (!std::filesystem::is_directory(path))
				icon = &fileIcon;

			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, iconBackgroundColor);
			if (ImGui::ImageButton(path.string().c_str(), *icon, ImVec2{iconSize, iconSize}))
			{
				
			}
			ImGui::PopStyleColor();
			if (std::filesystem::is_directory(path) && 
				ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				SH_INFO("double");
				ImGui::EndGroup();
				currentPath = path;
				GetAllFiles(currentPath);
				break;
			}
			std::string name = GetElideFileName(path, iconSize);
			float textWidth = ImGui::CalcTextSize(name.c_str()).x;
			float textOffset = (iconSize - textWidth) / 2.f;
			if (textOffset >= 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
			ImGui::Text("%s", name.c_str());
			ImGui::EndGroup();

			ImGui::SameLine(0.0f, spacing);
			cursorX += iconSize + spacing;
		}
		
		ImGui::End();
	}
}