#pragma once

#include "Export.h"

#include <string>
#include <string_view>
#include <optional>

namespace sh::editor
{
	class Project
	{
	private:
		std::string dir;
	public:
		static bool IsProject(std::string_view dir);
		static auto NewProject(std::string_view dir) -> Project;
		static auto OpenProject(std::string_view dir) -> std::optional<Project>;
	};
}