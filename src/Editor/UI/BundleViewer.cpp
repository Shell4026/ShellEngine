#include "UI/BundleViewer.h"
#include "imgui.h"
#include "Core/FileSystem.h"
#include "Core/Util.h"

namespace sh::editor
{
    BundleViewer::BundleViewer()
        : bShow(false)
    {
        explorer = std::make_unique<ExplorerUI>();
    }

    BundleViewer::~BundleViewer() = default;

    SH_EDITOR_API void BundleViewer::Render()
    {
        if (bShow)
        {
            ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_::ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Bundle Viewer", &bShow))
            {
                if (ImGui::Button("Open Bundle File"))
                {
                    explorer->PushCallbackQueue(
                        [this](const std::filesystem::path& path) 
                        {
                            bundle.LoadBundle(path);
                        }
                    );
                    explorer->Open();
                }

                if (bundle.IsLoaded())
                {
                    ImGui::Separator();
                    ShowBundleContent();
                }
            }
            ImGui::End();
        }
        else
        {
            if (bundle.IsLoaded())
                bundle.Clear();
        }
        explorer->Render();
    }

    SH_EDITOR_API void BundleViewer::Open()
    {
        bShow = true;
    }

    void BundleViewer::ShowBundleContent()
    {
        ImGui::Text("Version: %u", bundle.GetVersion());

        if (ImGui::BeginTable("AssetEntries", 5,
            ImGuiTableFlags_::ImGuiTableFlags_Borders | ImGuiTableFlags_::ImGuiTableFlags_RowBg | ImGuiTableFlags_::ImGuiTableFlags_ScrollX))
        {
            ImGui::TableSetupColumn("UUID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Offset");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Compressed");
            ImGui::TableHeadersRow();

            for (const auto& [uuid, entry] : bundle.GetAssetEntries())
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", uuid.ToString().c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", std::string{ entry.type }.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.dataOffset);
                ImGui::TableNextColumn();
                ImGui::Text("%u", entry.dataSize);
                ImGui::TableNextColumn();
                ImGui::Text(entry.bCompressed ? "true" : "false");
            }
            ImGui::EndTable();
        }
    }
}//namespace
