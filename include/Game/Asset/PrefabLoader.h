#pragma once
#include "Export.h"

#include "Core/IAssetLoader.h"

#include <filesystem>
namespace sh::game
{
	class PrefabLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API PrefabLoader();

		SH_GAME_API auto Load(const std::filesystem::path& filePath) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}//namespace