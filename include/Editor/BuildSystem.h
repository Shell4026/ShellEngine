#pragma once
#include "Export.h"

#include "Core/ISerializable.h"
#include "Core/UUID.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <filesystem>
namespace sh::core
{
	class AssetBundle;
}
namespace sh::game
{
	class World;
	class GameObject;
}
namespace sh::editor
{
	class Project;

	class BuildSystem
	{
	public:
		SH_EDITOR_API BuildSystem();

		SH_EDITOR_API void Build(Project& project, const std::filesystem::path& outputPath);
	private:
		void ExtractUUIDs(std::unordered_set<std::string>& set, const core::Json& world);
		void PackingAssets(core::AssetBundle& bundle, game::World& world);
		void ExportGameManager(const std::filesystem::path& outputPath);
	private:
		Project* currentProject = nullptr;

		std::regex uuidRegex;

		std::unordered_set<std::string> uuids;
		std::unordered_map<std::string, std::vector<std::string>> worldUUIDs;
	};
}//namespace