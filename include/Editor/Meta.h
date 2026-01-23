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
		constexpr static int VERSION = 1;
	public:
		SH_EDITOR_API Meta();

		SH_EDITOR_API auto Load(const std::filesystem::path& path) -> bool;
		SH_EDITOR_API auto DeserializeSObject(core::SObject& obj) const -> bool;

		SH_EDITOR_API void SaveWithObj(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash = true);
		SH_EDITOR_API void Save(const core::SObject& obj, const std::filesystem::path& path, bool bCalcHash = true);

		SH_EDITOR_API auto IsLoad() const -> bool;
		SH_EDITOR_API auto IsChanged() const -> bool;
		SH_EDITOR_API auto HasObjData() const -> bool;

		SH_EDITOR_API auto GetUUID() const -> const core::UUID&;
		SH_EDITOR_API auto GetTypeHash() const -> std::size_t;
		SH_EDITOR_API auto GetName() const -> const std::string&;
		SH_EDITOR_API auto GetObjJson() const -> const core::Json*;

		SH_EDITOR_API void Clear();

		SH_EDITOR_API static auto CreateMetaDirectory(const std::filesystem::path& filePath) -> std::filesystem::path;
	private:

		std::string name;
		std::size_t typeHash;
		core::UUID uuid;
		std::filesystem::path path;
		core::Json json;
		std::size_t hash = 0;

		bool bChanged = false;
	};
}//namespace