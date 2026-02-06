#pragma once
#include "Game/Export.h"

#include "Core/ModuleLoader.h"
#include "Core/Plugin.h"

#include <filesystem>
#include <vector>
namespace sh::game
{
	class ComponentLoader
	{
	public:
		struct ComponentInfo
		{
			std::string name;
			const core::reflection::STypeInfo* type;
		};
	public:
		/// @brief 플러그인을 불러와 컴포넌트에 등록한다. 확장자는 OS별로 다르므로 적으면 안 된다.
		/// @param pluginPath 컴포넌트 파일 경로
		SH_GAME_API void LoadPlugin(const std::filesystem::path& pluginPath);
		SH_GAME_API void UnloadPlugin();
		SH_GAME_API auto GetLoadedComponents() const -> const std::vector<ComponentInfo>& { return userComponents; }
		SH_GAME_API auto GetLoadedPluginPath() const -> const std::filesystem::path& { return plugin.path; }
		SH_GAME_API auto IsLoaded() const -> bool { return plugin.handle != nullptr; }

		/// @brief 확장자가 없는 플러그인 경로를 OS에 맞는 확장자를 붙여서 반환 해주는 함수.
		/// @param pluginPath 확장자 없는 플러그인 경로
		/// @return 확장자가 붙은 플러그인 경로
		SH_GAME_API static auto CreatePluginPath(const std::filesystem::path& pluginPath) -> std::filesystem::path;
	private:
		
		core::Plugin plugin;

		std::vector<ComponentInfo> userComponents;

		core::ModuleLoader moduleLoader;
	};
}//namespace