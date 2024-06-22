#include "Project.h"

#include <filesystem>

namespace sh::editor
{
	bool Project::IsProject(std::string_view dir)
	{
		return false;
	}

	auto Project::NewProject(std::string_view dir) -> Project
	{
		return Project{};
	}

	auto Project::OpenProject(std::string_view dir) -> std::optional<Project>
	{
		if (!std::filesystem::is_directory(dir))
			return {};
		return {};
	}
}