#include "ExplorerUI.h"
#include "EditorResource.h"

#include "Core/FileSystem.h"

#include <filesystem>
#include <algorithm>
namespace sh::editor
{
    SH_EDITOR_API ExplorerUI::ExplorerUI(game::ImGUImpl& imgui) :
        UI(imgui),
		currentPath(std::filesystem::current_path())
	{

	}

	void ExplorerUI::UpdateDirectoryEntries()
	{
        folders.clear();
        files.clear();
		for (const auto& entry : std::filesystem::directory_iterator(currentPath)) 
        {
            if (std::filesystem::is_directory(entry))
                folders.push_back(entry.path().filename());
            else
                files.push_back(entry.path().filename());
			//directoryEntries.push_back(entry.path().filename().string());
		}
        std::sort(folders.begin(), folders.end());
        std::sort(files.begin(), files.end());
	}

    inline void ExplorerUI::DrawFolderIcon()
    {
        float height = ImGui::GetTextLineHeight();
        auto folderIcon = EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Folder);
        ImGui::Image(*folderIcon, ImVec2{ height , height });
    }
    inline void ExplorerUI::RenderLeftSide()
    {
        static ImGuiChildFlags childFlags = ImGuiChildFlags_::ImGuiChildFlags_ResizeX | ImGuiChildFlags_::ImGuiChildFlags_Border;
        ImGui::BeginChild("Menus", ImVec2{ 100, 0 }, childFlags);
        if (ImGui::Selectable("Home"))
            currentPath = core::FileSystem::GetHomeDirectory();
        if (ImGui::Selectable("Desktop"))
            currentPath = core::FileSystem::GetDesktopDirectory();
        ImGui::EndChild();
    }
    inline void ExplorerUI::RenderRightSide()
    {
        ImGui::BeginGroup();
        // 주소창
        std::string path = currentPath.u8string();
        if (ImGui::InputText("Path", &path, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::filesystem::path newPath(path);
            if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath))
            {
                currentPath = newPath;
                UpdateDirectoryEntries();
            }
        }
        ImGui::Separator();

        float height = ImGui::GetContentRegionAvail().y;
        ImGui::BeginChild("ExplorerChild", ImVec2{ 0, height - 30 });
        // 디렉토리 목록
        if (std::filesystem::path(currentPath).has_parent_path())
        {
            DrawFolderIcon();
            ImGui::SameLine();
            ImGui::Selectable("../", false, ImGuiSelectableFlags_DontClosePopups);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
            {
                currentPath = std::filesystem::path(currentPath).parent_path();
            }
        }
        for (const auto& folderName : folders)
        {
            DrawFolderIcon();
            ImGui::SameLine();
            ImGui::Selectable(folderName.u8string().c_str(), false, ImGuiSelectableFlags_DontClosePopups);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
            {
                std::filesystem::path newPath = std::filesystem::path(currentPath) / folderName;
                if (std::filesystem::is_directory(newPath))
                {
                    currentPath = newPath;
                }
            }
        }
        for (const auto& fileName : files)
        {
            if (ImGui::Selectable(fileName.u8string().c_str(), false, ImGuiSelectableFlags_DontClosePopups))
                selected = fileName;
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
            {
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        if (mode == OpenMode::Select)
        {
            static std::string name = selected.u8string();
            ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            if (ImGui::Button("Select"))
            {
                while (!callbacks.empty())
                {
                    auto& func = callbacks.front();
                    if (selected.empty())
                        func(currentPath);
                    else
                        func(currentPath / selected);
                    callbacks.pop();
                }
                bOpen = false;
            }
        }
        else if (mode == OpenMode::Create)
        {
            static std::string name{};
            ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            if (ImGui::Button("Create"))
            {
                while (!callbacks.empty())
                {
                    auto& func = callbacks.front();
                    if (name.empty())
                        func(currentPath);
                    else
                        func(currentPath / name);
                    callbacks.pop();
                }
                bOpen = false;
            }
        }

        ImGui::EndGroup();
    }

    SH_EDITOR_API void ExplorerUI::Update()
	{
	}
    SH_EDITOR_API void ExplorerUI::Render()
    {
        if (!bOpen)
            return;
        UpdateDirectoryEntries();
        ImGui::Begin("File Explorer", &bOpen, ImGuiWindowFlags_::ImGuiWindowFlags_NoDocking);

        RenderLeftSide();
        ImGui::SameLine();
        RenderRightSide();

        ImGui::End();
    }
    SH_EDITOR_API void ExplorerUI::Open(OpenMode mode)
    {
        this->mode = mode;
        bOpen = true;
    }
    SH_EDITOR_API void ExplorerUI::AddCallback(const std::function<void(std::filesystem::path dir)>& func)
    {
        callbacks.push(func);
    }
}//namespace