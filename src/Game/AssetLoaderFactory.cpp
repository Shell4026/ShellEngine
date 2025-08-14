#include "AssetLoaderFactory.h"

namespace sh::game
{
	SH_GAME_API void AssetLoaderFactory::RegisterLoader(const std::string& type, std::unique_ptr<core::IAssetLoader> loader)
	{
		loaders.emplace(type, std::move(loader));
	}
	SH_GAME_API auto AssetLoaderFactory::GetLoader(const std::string& type) -> core::IAssetLoader*
	{
		auto it = loaders.find(type);
		if (it != loaders.end())
		{
			return it->second.get();
		}
		return nullptr;
	}
}
