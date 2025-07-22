#include "Meta.h"

#include "Core/SObject.h"
#include "Core/FileSystem.h"
#include "Core/Logger.h"

#include <cassert>
namespace sh::editor
{
	Meta::Meta() :
		uuid(core::UUID::Generate())
	{
	}
	SH_EDITOR_API auto Meta::Load(const std::filesystem::path& path) -> bool
	{
		if (!std::filesystem::exists(path))
			return false;

		this->path = path;

		auto metaText = core::FileSystem::LoadText(path);
		if (!metaText.has_value())
		{
			SH_ERROR_FORMAT("Can't read {}", path.u8string());
			return false;
		}
		json = core::Json::parse(metaText.value());
		
		if (!json.contains("metaHash"))
		{
			SH_ERROR_FORMAT("Not found metaHash key from {}", path.u8string());
			json.clear();
			return false;
		}
		hash = json["metaHash"];

		if (!json.contains("obj"))
		{
			SH_ERROR_FORMAT("Not found obj key from {}", path.u8string());
			json.clear();
			return false;
		}
		core::Json objJson{};
		objJson = json["obj"];

		std::hash<std::string> hasher{};
		std::size_t objHash = hasher(objJson.dump());

		bChanged = hash != objHash;

		if (!objJson.contains("uuid"))
		{
			SH_ERROR_FORMAT("Not found UUID from {}", path.u8string());
			json.clear();
			return false;
		}
		uuid = core::UUID{ objJson["uuid"].get<std::string>() };
		return true;
	}
	SH_EDITOR_API auto Meta::DeserializeSObject(core::SObject& obj) const -> bool
	{
		assert(IsLoad());
		obj.Deserialize(json["obj"]);
		return true;
	}
	SH_EDITOR_API void Meta::Save(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash)
	{
		core::Json metaJson{};
		metaJson["obj"] = obj.Serialize();

		if (bCalcHash)
		{
			std::hash<std::string> hasher{};
			metaJson["metaHash"] = hasher(metaJson["obj"].dump());
		}
		else
			metaJson["metaHash"] = hash;

		std::ofstream os{ path };
		if (os.is_open())
		{
			os << std::setw(4) << metaJson;
			os.close();
		}
		else
		{
			SH_ERROR_FORMAT("Can't create meta: {}", path.u8string());
		}
	}
	SH_EDITOR_API auto Meta::IsLoad() const -> bool
	{
		return !json.empty();
	}
	SH_EDITOR_API auto Meta::GetUUID() const -> const core::UUID&
	{
		return uuid;
	}
	SH_EDITOR_API auto Meta::IsChanged() const -> bool
	{
		return bChanged;
	}
}//namespace