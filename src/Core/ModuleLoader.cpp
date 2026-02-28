#include "ModuleLoader.h"
#include "Logger.h"

#include <iostream>
#include <functional>
#if _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif
namespace sh::core
{
	auto ModuleLoader::Load(const std::filesystem::path& moduleDir) -> std::optional<Plugin>
	{
#if _WIN32
		HMODULE handle = LoadLibraryExA(TEXT(moduleDir.string().c_str()), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (handle == nullptr)
		{
			SH_ERROR_FORMAT("Error load module: {}", moduleDir.u8string());
			return {};
		}

		std::function<void()> fnInit{ GetProcAddress(handle, "Init") };
		if (fnInit)
			fnInit();

		std::function<Plugin()> fnGetPluginInfo = reinterpret_cast<Plugin(*)()>(GetProcAddress(handle, "GetPluginInfo"));
		Plugin plugin{};
		if (fnGetPluginInfo)
		{
			plugin = fnGetPluginInfo();
		}
#elif __linux__
		void* handle = dlopen(moduleDir.c_str(), RTLD_LAZY);
		if (!handle) {
			SH_ERROR_FORMAT("Error load module: {}", moduleDir.u8string());
			return {};
		}

		std::function<void()> fnInit{ reinterpret_cast<void*(*)()>(dlsym(handle, "Init")) };
		fnInit();

		Plugin(*fnGetPluginInfo)() = (Plugin(*)())dlsym(handle, "GetPluginInfo");
		Plugin plugin{};
		if (fnGetPluginInfo)
		{
			plugin = fnGetPluginInfo();
		}
#endif
		plugin.handle = handle;
		plugin.path = moduleDir;
		return plugin;
	}
	SH_CORE_API void ModuleLoader::Clean(const Plugin& plugin)
	{
		if (plugin.handle == nullptr)
			return;
#if _WIN32
		auto result = FreeLibrary(reinterpret_cast<HMODULE>(plugin.handle));
		assert(result);
#elif __linux__
		auto result = dlclose(plugin.handle);
		assert(result);
#endif
	}
}