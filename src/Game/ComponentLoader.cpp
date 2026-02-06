#include "ComponentLoader.h"
#include "ComponentModule.h"

#include "Core/Logger.h"

namespace sh::game
{
	SH_GAME_API void ComponentLoader::LoadPlugin(const std::filesystem::path& pluginPath)
	{
		const std::filesystem::path actualPath = CreatePluginPath(pluginPath);

		auto plugin = moduleLoader.Load(actualPath);
		if (!plugin.has_value())
			SH_ERROR_FORMAT("Failed to load module: {}", actualPath.u8string());
		else
			this->plugin = std::move(plugin.value());

		static auto& componentModule = *ComponentModule::GetInstance();
		for (const auto& componentInfo : componentModule.GetWaitingComponents())
			userComponents.push_back({ componentInfo.name, &componentInfo.type });

		componentModule.RegisterWaitingComponents();
	}
	SH_GAME_API void ComponentLoader::UnloadPlugin()
	{
		moduleLoader.Clean(plugin);
		plugin.handle = nullptr;
		userComponents.clear();
	}
	SH_GAME_API auto ComponentLoader::CreatePluginPath(const std::filesystem::path& pluginPath) -> std::filesystem::path
	{
		std::filesystem::path dllPath = pluginPath;
#if _WIN32
		if (pluginPath.has_extension())
		{
			if (pluginPath.extension() != ".dll")
				dllPath = pluginPath.parent_path() / std::filesystem::u8path(pluginPath.stem().u8string() + ".dll");
		}
		else
			dllPath = std::filesystem::u8path(pluginPath.u8string() + ".dll");

		auto pdbPath = pluginPath.parent_path() / std::filesystem::path(pluginPath.stem().u8string() + ".pdb");
		if (std::filesystem::exists(pdbPath))
			std::filesystem::remove(pdbPath);
#elif __linux__
		if (pluginPath.has_extension())
		{
			if (pluginPath.extension() != ".so")
				dllPath = pluginPath.parent_path() / std::filesystem::u8path("lib" + pluginPath.stem().u8string() + ".so");
		}
		else
			dllPath = std::filesystem::current_path() / pluginPath.parent_path() / std::filesystem::u8path("lib" + pluginPath.stem().u8string() + ".so");
#endif
		return dllPath;
	}
}//namespace