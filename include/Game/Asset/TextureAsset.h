#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/Texture.h"

#include <vector>
namespace sh::game
{
	/// @brief 텍스쳐 에셋 클래스
	class TextureAsset : public core::Asset
	{
		SASSET(TextureAsset, "tex")
	public:
		struct TextureHeader
		{
			uint32_t width;
			uint32_t height;
			render::Texture::TextureFormat format;
			uint32_t aniso;
			uint32_t filter;
			bool bMipmap;
		};
		struct PixelData
		{
			const uint8_t* pixelDataPtr;
			std::size_t size;
		};
	private:
		core::SObjWeakPtr<const render::Texture> texturePtr;
		TextureHeader header;
	private:
		void SetHeader(TextureHeader& header) const;
	public:
		constexpr static const char* ASSET_NAME = "tex";
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		SH_GAME_API TextureAsset();
		SH_GAME_API TextureAsset(const render::Texture& texture);
		SH_GAME_API ~TextureAsset();

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetHeader() const -> TextureHeader;
		SH_GAME_API auto GetPixelData() const -> PixelData;
	};
}//namespace