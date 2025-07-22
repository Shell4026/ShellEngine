#pragma once
#include "Export.h"

#include "Core/UUID.h"
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
		core::UUID uuid;
		std::filesystem::path path;
		core::Json json;
		std::size_t hash = 0;

		bool bChanged = false;
	public:
		SH_EDITOR_API Meta();

		SH_EDITOR_API auto Load(const std::filesystem::path& path) -> bool;
		SH_EDITOR_API auto DeserializeSObject(core::SObject& obj) const -> bool;

		SH_EDITOR_API void Save(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash = true);

		SH_EDITOR_API auto IsLoad() const -> bool;

		SH_EDITOR_API auto GetUUID() const -> const core::UUID&;
		SH_EDITOR_API auto IsChanged() const -> bool;
	};
}//namespace