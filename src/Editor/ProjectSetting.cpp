#include "ProjectSetting.h"
#include "AssetDatabase.h"

#include "Core/Util.h"
#include "Core/Logger.h"
#include "Core/FileSystem.h"

#include "Game/GameManager.h"

#include <fstream>
#include <optional>
namespace sh::editor
{
	ProjectSetting::ProjectSetting() :
		version(0),
		startingWorldUUID(core::UUID::GenerateEmptyUUID()),
		lastWorldUUID(core::UUID::GenerateEmptyUUID())
	{
	}
	ProjectSetting::~ProjectSetting()
	{
	}
	SH_EDITOR_API auto ProjectSetting::Serialize() const -> core::Json
	{
		core::Json json;
		json["version"] = version;
		json["startingWorld"] = startingWorldUUID.ToString();
		json["lastWorld"] = lastWorldUUID.ToString();
		return json;
	}
	SH_EDITOR_API void ProjectSetting::Deserialize(const core::Json& json)
	{
		if (json.contains("version"))
			version = json["version"];
		if (json.contains("startingWorld"))
			startingWorldUUID = core::UUID{ json["startingWorld"].get_ref<const std::string&>() };
		if (json.contains("lastWorld"))
			lastWorldUUID = core::UUID{ json["lastWorld"].get_ref<const std::string&>() };
	}
	SH_EDITOR_API void ProjectSetting::Save(const std::filesystem::path& path)
	{
		SH_INFO_FORMAT("Save project setting: {}", path.u8string());
		if (!core::FileSystem::SaveText(Serialize(), path))
			SH_ERROR_FORMAT("Save error! {}", path.u8string());
	}
	SH_EDITOR_API void ProjectSetting::Load(const std::filesystem::path& path)
	{
		auto stringOpt = core::FileSystem::LoadText(path);
		if (stringOpt.has_value())
		{
			if (stringOpt.value().empty())
				Save(path);
			else
				Deserialize(core::Json::parse(stringOpt.value()));
		}
		else
			Save(path);
	}
}
