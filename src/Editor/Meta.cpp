#include "Meta.h"

#include "Core/SObject.h"
#include "Core/FileSystem.h"
#include "Core/Logger.h"

#include <cassert>
namespace sh::editor
{
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
		data = std::move(metaText.value());
		json = core::Json::parse(data);
		return true;
	}
	SH_EDITOR_API auto Meta::LoadImporter(IImporter& importer) const -> bool
	{
		assert(IsLoad());
		if (json.contains(importer.GetName()))
			importer.Deserialize(json[importer.GetName()]);
		else
		{
			SH_ERROR_FORMAT("Can't read importer setting: {}", path.u8string());
			return false;
		}
		return true;
	}
	SH_EDITOR_API auto Meta::LoadSObject(core::SObject& obj) const -> bool
	{
		assert(IsLoad());
		obj.Deserialize(json);
		return true;
	}
	SH_EDITOR_API void Meta::Save(const core::SObject& obj, const IImporter& importer, const std::filesystem::path& path)
	{
		core::Json metaJson = obj.Serialize();
		metaJson[importer.GetName()] = importer.Serialize();
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
		return !data.empty();
	}
}//namespace