#pragma once
#include "Editor/Export.h"

#include <filesystem>
namespace sh::editor
{
	class ProjectSetting;
	class ProjectSettingUI
	{
	public:
		SH_EDITOR_API static void RenderUI(ProjectSetting& setting, const std::filesystem::path& rootPath, bool& bOpen);
	};
}//namespace