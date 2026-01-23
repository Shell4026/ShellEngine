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
		
		if (!json.contains("uuid"))
		{
			SH_ERROR_FORMAT("Not found UUID from {}", path.u8string());
			json.clear();
			return false;
		}
		if (!json.contains("typeHash"))
		{
			SH_ERROR_FORMAT("Not found typeHash from {}", path.u8string());
			json.clear();
			return false;
		}
		if (!json.contains("name"))
		{
			SH_ERROR_FORMAT("Not found name from {}", path.u8string());
			json.clear();
			return false;
		}
		if (json.contains("metaHash"))
		{
			hash = json["metaHash"];
			if (json.contains("obj"))
			{
				const core::Json objJson = json["obj"];
				std::hash<std::string> hasher{};
				const std::size_t objHash = hasher(objJson.dump());

				bChanged = hash != objHash;
			}
		}
		uuid = core::UUID{ json["uuid"].get_ref<const std::string&>() };
		typeHash = json["typeHash"];
		name = json["name"];
		return true;
	}
	SH_EDITOR_API auto Meta::DeserializeSObject(core::SObject& obj) const -> bool
	{
		assert(IsLoad());
		if (json.contains("uuid"))
			obj.SetUUID(core::UUID{ json["uuid"].get_ref<const std::string&>() });
		if (!json.contains("obj"))
			return false;
		obj.Deserialize(json["obj"]);
		return true;
	}
	SH_EDITOR_API void Meta::SaveWithObj(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash)
	{
		core::Json metaJson{};
		metaJson["uuid"] = obj.GetUUID().ToString();
		metaJson["typeHash"] = obj.GetType().type.hash;
		metaJson["name"] = obj.GetName().ToString();
		metaJson["version"] = 1;

		if (bCalcHash)
		{
			std::hash<std::string> hasher{};
			metaJson["metaHash"] = hasher(obj.Serialize().dump());
		}
		else
			metaJson["metaHash"] = hash;

		metaJson["obj"] = obj.Serialize();

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
	SH_EDITOR_API void Meta::Save(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash)
	{
		core::Json metaJson{};
		metaJson["uuid"] = obj.GetUUID().ToString();
		metaJson["typeHash"] = obj.GetType().type.hash;
		metaJson["name"] = obj.GetName().ToString();
		metaJson["version"] = VERSION;

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
	SH_EDITOR_API auto Meta::HasObjData() const -> bool
	{
		if (!IsLoad())
			return false;
		return json.contains("obj");
	}
	SH_EDITOR_API auto Meta::GetUUID() const -> const core::UUID&
	{
		return uuid;
	}
	SH_EDITOR_API auto Meta::GetTypeHash() const -> std::size_t
	{
		return typeHash;
	}
	SH_EDITOR_API auto Meta::GetName() const -> const std::string&
	{
		return name;
	}
	SH_EDITOR_API auto Meta::GetObjJson() const -> const core::Json*
	{
		if (!HasObjData())
			return nullptr;
		return &json["obj"];
	}
	SH_EDITOR_API void Meta::Clear()
	{
		return json.clear();
	}
	SH_EDITOR_API auto Meta::IsChanged() const -> bool
	{
		return bChanged;
	}
	SH_EDITOR_API auto Meta::CreateMetaDirectory(const std::filesystem::path& filePath) -> std::filesystem::path
	{
		std::filesystem::path metaPath = filePath;
		metaPath += ".meta";
		return metaPath;
	}
}//namespace