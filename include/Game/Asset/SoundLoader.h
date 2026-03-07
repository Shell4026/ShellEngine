#pragma once
#include "../Export.h"

#include "Core/IAssetLoader.h"

namespace sh::game
{
	class SoundLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API SoundLoader();

		SH_GAME_API auto Load(const std::filesystem::path& filePath) const -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) const -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}//namespace