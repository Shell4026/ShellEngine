#include "ExplorerUI.h"

#include <filesystem>
namespace sh::editor
{
	ExplorerUI::ExplorerUI(game::ImGUImpl& imgui) :
        UI(imgui),
		currentPath(std::filesystem::current_path().string())
	{

	}

	void ExplorerUI::UpdateDirectoryEntries()
	{
		directoryEntries.clear();
		for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
			directoryEntries.push_back(entry.path().filename().string());
		}
	}

	void ExplorerUI::Update()
	{
        UpdateDirectoryEntries();
	    ImGui::Begin("File Explorer");

        // 주소창
        char pathBuffer[256];
        strncpy(pathBuffer, currentPath.c_str(), sizeof(pathBuffer));
        if (ImGui::InputText("Path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::filesystem::path newPath(pathBuffer);
            if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
                currentPath = newPath.string();
                UpdateDirectoryEntries();
            }
        }

        // 디렉토리 목록
        if (std::filesystem::path(currentPath).has_parent_path())
        {
            if (ImGui::Selectable("../", false, ImGuiSelectableFlags_DontClosePopups)) {
                currentPath = std::filesystem::path(currentPath).parent_path().string();
            }
        }
        for (const auto& entry : directoryEntries) {
            if (ImGui::Selectable(entry.c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
                std::filesystem::path newPath = std::filesystem::path(currentPath) / entry;
                if (std::filesystem::is_directory(newPath)) {
                    currentPath = newPath.string();
                }
            }
        }

        ImGui::End();
	}
    void ExplorerUI::Render()
    {
    }
}