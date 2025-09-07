#pragma once
#include "../Export.h"
#include "../Prefab.h"

#include "Core/Asset.h"
#include "Core/ISerializable.h"

namespace sh::game
{
	class PrefabAsset : public core::Asset
	{
		SASSET(PrefabAsset, "pref")
	public:
		SH_GAME_API PrefabAsset();
		SH_GAME_API PrefabAsset(const Prefab& prefab);
		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetPrefabData() const -> const core::Json&;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "pref";
	private:
		core::Json prefabData;
		const Prefab* prefabPtr = nullptr;
	};
}
