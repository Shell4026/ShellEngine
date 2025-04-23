#pragma once
#include "Export.h"
#include "Plugin.h"

#include <filesystem>
#include <optional>
namespace sh::core
{
	class ModuleLoader
	{
	public:
		SH_CORE_API auto Load(const std::filesystem::path& moduleDir) -> std::optional<Plugin>;
		SH_CORE_API void Clean(const Plugin& plugin);
	};
}