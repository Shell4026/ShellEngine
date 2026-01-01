#pragma once
#include "Editor/Export.h"
#include "ExplorerUI.h"

#include "Core/AssetBundle.h"
#include "Core/NonCopyable.h"

#include <memory>
#include <vector>
namespace sh::editor
{
    class BundleViewer : public sh::core::INonCopyable
    {
    public:
        SH_EDITOR_API BundleViewer();
        SH_EDITOR_API ~BundleViewer();

        SH_EDITOR_API void Render();
        SH_EDITOR_API void Open();

        SH_EDITOR_API auto GetExplorer() const -> ExplorerUI*;
    private:
        void ShowBundleContent();
    private:
        sh::core::AssetBundle bundle;
        std::unique_ptr<ExplorerUI> explorer;

        struct AssetInfo
        {
            std::string uuid;
            std::string type;
            uint64_t offset;
            uint64_t size;
            bool bCompressed;
        };
        std::vector<AssetInfo> assets;

        bool bShow;
    };
}//namespace