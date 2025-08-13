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
    private:
        void ShowBundleContent();
    private:
        sh::core::AssetBundle bundle;
        std::unique_ptr<ExplorerUI> explorer;

        bool bShow;
    };
}//namespace