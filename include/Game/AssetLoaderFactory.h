#pragma once
#include "Game/Export.h"
#include "Core/Singleton.hpp"
#include "Core/IAssetLoader.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace sh::game
{
	class AssetLoaderFactory : public core::Singleton<AssetLoaderFactory>
	{
		friend class core::Singleton<AssetLoaderFactory>;
	private:
		AssetLoaderFactory() = default;

	public:
		void RegisterLoader(const std::string& type, std::unique_ptr<core::IAssetLoader> loader);
		auto GetLoader(const std::string& type) -> core::IAssetLoader*;

	private:
		std::unordered_map<std::string, std::unique_ptr<core::IAssetLoader>> loaders;
	};
}
