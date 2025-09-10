#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/ISerializable.h"

namespace sh::game
{
	class World;

	class WorldAsset : public core::Asset
	{
		SASSET(WorldAsset, "worl")
	public:
		SH_GAME_API WorldAsset();
		SH_GAME_API WorldAsset(const World& world);
		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API void ConvertToGameWorldType(bool bConvert = true);

		SH_GAME_API auto GetWorldData() const -> const core::Json&;
		SH_GAME_API auto ReleaseWorldData() -> core::Json&&;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "worl";
	private:
		core::Json worldData;
		const World* worldPtr = nullptr;
		
		bool bConvertWorldType = false;
	};
}
