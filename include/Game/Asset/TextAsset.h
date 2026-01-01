#pragma once
#include "../Export.h"
#include "../TextObject.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include <vector>
namespace sh::game
{
	class TextAsset : public core::Asset
	{
		SASSET(TextAsset, "text")
	public:
		SH_GAME_API TextAsset();
		SH_GAME_API TextAsset(const TextObject& textObj);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetRawData() const -> const std::vector<uint8_t>& { return data; }
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "text";
	private:
		core::SObjWeakPtr<const TextObject> textObjPtr;
	};
}
