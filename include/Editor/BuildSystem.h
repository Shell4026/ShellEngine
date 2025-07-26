#pragma once
#include "Export.h"

#include "Core/ISerializable.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <filesystem>
namespace sh::game
{
	class World;
}
namespace sh::editor
{
	class Project;

	class BuildSystem
	{
	private:
		Project* currentProject = nullptr;

		std::regex uuidRegex;

		std::unordered_set<std::string> uuids;

		void ExtractUUIDs(const core::Json& world);
		void ExtractUUID(const core::Json& node);
		void PackingAssets(const std::filesystem::path& outputPath);
	public:
		SH_EDITOR_API BuildSystem();

		SH_EDITOR_API void Build(Project& project, game::World& world, const std::filesystem::path& outputPath);
	};
}//namespace