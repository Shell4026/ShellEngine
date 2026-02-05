#include "UI/ExplorerUI.h"
#include "EditorResource.h"
#include "EditorWorld.h"

#include "Core/FileSystem.h"
#include "Core/Logger.h"

#include <filesystem>
#include <algorithm>
namespace sh::editor
{
    SH_EDITOR_API ExplorerUI::ExplorerUI() :
		currentPath(std::filesystem::current_path())
	{
	}

    SH_EDITOR_API void ExplorerUI::SetLatestPath(const std::filesystem::path& path)
    {
        latest = path;
    }

    SH_EDITOR_API void ExplorerUI::ResetSelected()
    {
        selected.clear();
    }

    void ExplorerUI::UpdateDirectoryEntries()
	{
        if (!std::filesystem::exists(currentPath))
            return;
        auto a = {1, 2 ,3};
        folders.clear();
        files.clear();
		for (const auto& entry : std::filesystem::directory_iterator(currentPath)) 
        {
            if (std::filesystem::is_directory(entry))
            {
                folders.push_back(entry.path().filename());
            }
            else
            {
                if (flag & FlagEnum::FolderOnly)
                    continue;
                if (extensionFilters.empty())
                    files.push_back(entry.path().filename());
                else
                {

                    auto it = std::find(extensionFilters.begin(), extensionFilters.end(), entry.path().extension().u8string());
                    if (it != extensionFilters.end())
                        files.push_back(entry.path().filename());
                }
            }
			//directoryEntries.push_back(entry.path().filename().string());
		}
        std::sort(folders.begin(), folders.end());
        std::sort(files.begin(), files.end());
	}

    inline void ExplorerUI::DrawFolderIcon()
    {
        float height = ImGui::GetTextLineHeight();
        auto folderIcon = EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Folder);
        folderIcon->Draw(ImVec2{ height , height });
    }
    inline void ExplorerUI::RenderLeftSide()
    {
        static ImGuiChildFlags childFlags = ImGuiChildFlags_::ImGuiChildFlags_ResizeX | ImGuiChildFlags_::ImGuiChildFlags_Border;
        ImGui::BeginChild("Menus", ImVec2{ 100, 0 }, childFlags);
        if (ImGui::Selectable("Home"))
            currentPath = core::FileSystem::GetHomeDirectory();
        if (ImGui::Selectable("Desktop"))
            currentPath = core::FileSystem::GetDesktopDirectory();

        ImGui::Separator();

        if (!latest.empty())
        {
            ImGui::Text("Lastest");
            if (ImGui::Selectable(latest.filename().u8string().c_str()))
            {
                if (std::filesystem::exists(latest))
                    currentPath = latest;
                else
                    SH_ERROR_FORMAT("{} is not exists!", latest.u8string());
            }
        }
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
                SelectFile();
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        static std::string name{};
        name = selected.u8string();
        ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue);
        selected = std::filesystem::u8path(name);
        ImGui::SameLine();

        if (mode == OpenMode::Select)
        {
            if (ImGui::Button("Select"))
            {
                if (selected.empty() && std::filesystem::exists(currentPath))
                    SelectFile();
                else if (std::filesystem::exists(currentPath / selected))
                    SelectFile();
            }
        }
        else if (mode == OpenMode::Create)
        {
            if (ImGui::Button("Create"))
            {
                if (!name.empty())
                {
                    selected = std::filesystem::u8path(name);
                    SelectFile();
                }
            }
        }

        ImGui::EndGroup();
    }

    void ExplorerUI::SelectFile()
    {
        bOpen = false;
        while (!callbacks.empty())
        {
            auto& func = callbacks.front();
            if (selected.empty())
                func(currentPath);
            else
                func(currentPath / selected);
            callbacks.pop();
        }
    }

    SH_EDITOR_API void ExplorerUI::Update()
	{
	}
    SH_EDITOR_API void ExplorerUI::Render()
    {
        if (!bOpen)
            return;
        UpdateDirectoryEntries();
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_::ImGuiCond_Once);
        ImGui::Begin("File Explorer", &bOpen, ImGuiWindowFlags_::ImGuiWindowFlags_NoDocking);

        RenderLeftSide();
        ImGui::SameLine();
        RenderRightSide();

        ImGui::End();
    }
    SH_EDITOR_API void ExplorerUI::Open(OpenMode mode, Flag flag)
    {
        this->mode = mode;
        this->flag = flag;
        bOpen = true;
    }
    SH_EDITOR_API void ExplorerUI::SetCurrentPath(const std::filesystem::path& path)
    {
        currentPath = path;
    }
    SH_EDITOR_API void ExplorerUI::SetExtensionFilter(const std::string& extension)
    {
        ClearExtensionFilter();
        extensionFilters.push_back(extension);
    }
    SH_EDITOR_API void ExplorerUI::SetExtensionFilter(const std::initializer_list<std::string>& extensions)
    {
        ClearExtensionFilter();
        extensionFilters.assign(extensions);
    }
    SH_EDITOR_API void ExplorerUI::ClearExtensionFilter()
    {
        extensionFilters.clear();
    }
    SH_EDITOR_API void ExplorerUI::PushCallbackQueue(const std::function<void(std::filesystem::path dir)>& func)
    {
        callbacks.push(func);
    }
}//namespace