#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/Font.h"

#include <vector>
namespace sh::game
{
	class FontAsset : public core::Asset
	{
		SASSET(FontAsset, "font")
	public:
		SH_GAME_API FontAsset();
		SH_GAME_API FontAsset(const render::Font& font);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetFontJson() const -> const core::Json& { return fontJson; }
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "font";
	private:
		core::Json fontJson;
		core::SObjWeakPtr<const render::Font> fontPtr;
	};
}
