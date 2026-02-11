#pragma once
#include "Game/Export.h"
#include "Game/ScriptableObject.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include <vector>
namespace sh::game
{
	class ScriptableObjectAsset : public core::Asset
	{
		SASSET(ScriptableObjectAsset, "srpo")
	public:
		SH_GAME_API ScriptableObjectAsset();
		SH_GAME_API ScriptableObjectAsset(const ScriptableObject& obj);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetSerializationData() const -> const core::Json& { return serializationData; }
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "srpo";
	private:
		core::SObjWeakPtr<const ScriptableObject> objPtr;
		core::Json serializationData;
	};
}
