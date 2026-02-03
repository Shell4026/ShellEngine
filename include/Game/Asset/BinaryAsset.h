#pragma once
#include "../Export.h"
#include "../BinaryObject.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include <vector>
namespace sh::game
{
	class BinaryAsset : public core::Asset
	{
		SASSET(BinaryAsset, "bin")
	public:
		SH_GAME_API BinaryAsset();
		SH_GAME_API BinaryAsset(const BinaryObject& binaryObj);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetRawData() const -> const std::vector<uint8_t>& { return data; }
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "bin";
	private:
		core::SObjWeakPtr<const BinaryObject> binaryObjPtr;
	};
}
