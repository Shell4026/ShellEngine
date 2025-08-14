#include "ProjectSetting.h"

#include "Core/Util.h"

#include "Game/World.h"

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
		if (core::IsValid(startingWorld))
			json["startingWorld"] = startingWorld->GetUUID().ToString();
		return json;
	}
	SH_EDITOR_API void ProjectSetting::Deserialize(const core::Json& json)
	{
		if (json.contains("version"))
			version = json["version"];
		if (json.contains("startingWorld"))
		{
			core::SObject* worldObj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ json["startingWorld"].get<std::string>()});
			if (core::IsValid(worldObj))
				startingWorld = static_cast<game::World*>(worldObj);
		}
	}
}
