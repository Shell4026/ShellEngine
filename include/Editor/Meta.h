#pragma once
#include "Export.h"
#include "IImporter.h"

#include "Core/ISerializable.h"

#include <string>
#include <filesystem>
namespace sh::core
{
	class SObject;
}
namespace sh::editor
{
	class Meta
	{
	private:
		std::filesystem::path path;
		std::string data;
		core::Json json;
	public:
		SH_EDITOR_API auto Load(const std::filesystem::path& path) -> bool;
		SH_EDITOR_API auto LoadImporter(IImporter& importer) const -> bool;
		SH_EDITOR_API auto LoadSObject(core::SObject& obj) const -> bool;

		SH_EDITOR_API void Save(const core::SObject& obj, const IImporter& importer, const std::filesystem::path& path);

		SH_EDITOR_API auto IsLoad() const -> bool;
	};
}//namespace