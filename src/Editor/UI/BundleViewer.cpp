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
        explorer->SetExtensionFilter(".bundle");
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
                    assets.clear();
                    explorer->PushCallbackQueue(
                        [this](const std::filesystem::path& path) 
                        {
                            if (bundle.LoadBundle(path))
                            {
                                AssetInfo info{};
                                for (const auto& [uuid, entry] : bundle.GetAssetEntries())
                                {
                                    info.uuid = uuid.ToString();
                                    info.type = entry.type;
                                    info.offset = entry.dataOffset;
                                    info.size = entry.dataSize;
                                    info.bCompressed = entry.bCompressed;
                                    assets.push_back(info);
                                }
                            }
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

    SH_EDITOR_API auto BundleViewer::GetExplorer() const -> ExplorerUI*
    {
        return explorer.get();
    }

    void BundleViewer::ShowBundleContent()
    {
        ImGui::Text("Version: %u", bundle.GetVersion());

        if (ImGui::BeginTable("AssetEntries", 5,
            ImGuiTableFlags_::ImGuiTableFlags_Borders | ImGuiTableFlags_::ImGuiTableFlags_RowBg | ImGuiTableFlags_::ImGuiTableFlags_ScrollX | ImGuiTableFlags_::ImGuiTableFlags_Sortable))
        {
            ImGui::TableSetupColumn("UUID", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_None, 0.f, 0);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_DefaultSort, 0.f, 1);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_None, 0.f, 2);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_None, 0.f, 3);
            ImGui::TableSetupColumn("Compressed", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_NoSort);
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();

            if (sortSpecs != nullptr && sortSpecs->SpecsDirty)
            {
                std::sort(assets.begin(), assets.end(),
                    [sortSpecs](const AssetInfo& a, const AssetInfo& b)
                    {
                        for (int n = 0; n < sortSpecs->SpecsCount; n++) 
                        {
                            const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[n];

                            int delta = 0;
                            switch (spec.ColumnUserID) 
                            {
                            case 0:  
                                delta = a.uuid.compare(b.uuid); 
                                break;
                            case 1: 
                                delta = a.type.compare(b.type);
                                break;
                            case 2:    
                                delta = (a.offset < b.offset) ? -1 : (a.offset > b.offset) ? 1 : 0;
                                break;
                            case 3: 
                                delta = (a.size < b.size) ? -1 : (a.size > b.size) ? 1 : 0; 
                                break;
                            }

                            if (delta != 0) 
                            {
                                if (spec.SortDirection == ImGuiSortDirection::ImGuiSortDirection_Descending)
                                    delta = -delta;
                                return delta < 0;
                            }
                        }
                        return false;
                    });
            }

            for (const auto& assetInfo : assets)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", assetInfo.uuid.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", assetInfo.type.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%u", assetInfo.offset);
                ImGui::TableNextColumn();
                ImGui::Text("%u", assetInfo.size);
                ImGui::TableNextColumn();
                ImGui::Text(assetInfo.bCompressed ? "true" : "false");
            }
            ImGui::EndTable();
        }
    }
}//namespace
