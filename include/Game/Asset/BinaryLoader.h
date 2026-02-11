#pragma once
#include "../Export.h"

#include "Core/IAssetLoader.h"

#include <filesystem>

namespace sh::game
{
	class BinaryLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API BinaryLoader();

		SH_GAME_API auto Load(const std::filesystem::path& filePath) const -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) const -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}//namespace