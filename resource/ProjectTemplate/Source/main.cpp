#include "Export.h"

#include "Core/Logger.h"
#include "Core/Plugin.h"

#include "Game/ComponentModule.h"

extern "C"
{
	SH_USER_API void Init()
	{
		SH_INFO_FORMAT("Init User module: {}\n", SH_PROJECT_NAME);
	}
	SH_USER_API auto GetPluginInfo() -> sh::core::Plugin
	{
		sh::core::Plugin plugin{};
		plugin.name = SH_PROJECT_NAME;
		return plugin;
	}
}