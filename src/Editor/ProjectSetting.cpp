#include "ProjectSetting.h"

namespace sh::editor
{
	ProjectSetting::ProjectSetting() :
		version(0)
	{
	}
	SH_EDITOR_API auto ProjectSetting::Serialize() const -> core::Json
	{
		core::Json json;
		json["version"] = version;
		json["startingWorld"] = startingWorldPath.u8string();
		return json;
	}
	SH_EDITOR_API void ProjectSetting::Deserialize(const core::Json& json)
	{
		if (json.contains("version"))
			version = json["version"];
		if (json.contains("startingWorld"))
			startingWorldPath = std::filesystem::u8path(std::string{ json["startingWorld"] });
	}
}
