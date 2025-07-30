#pragma once
#include "Export.h"

#include "Core/Asset.h"
#include "Core/ISerializable.h"

namespace sh::game
{
	class World;

	class WorldAsset : public core::Asset
	{
		SASSET(WorldAsset, "worl")
	private:
		core::Json worldData;
		const World* worldPtr = nullptr;
	public:
		constexpr static const char* ASSET_NAME = "worl";
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		SH_GAME_API WorldAsset();
		SH_GAME_API WorldAsset(const World& world);
		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetWorldData() const -> const core::Json&;
		SH_GAME_API auto ReleaseWorldData() -> core::Json&&;
	};
}
