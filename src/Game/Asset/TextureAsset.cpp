#include "Asset/TextureAsset.h"
namespace sh::game
{
	void TextureAsset::SetHeader(TextureHeader& header) const
	{
		if (!texturePtr.IsValid())
			return;

		header.width = texturePtr->GetWidth();
		header.height = texturePtr->GetHeight();

		header.format = texturePtr->GetTextureFormat();
		header.aniso = texturePtr->GetAnisoLevel();
		header.bMipmap = texturePtr->GetMipLevel() > 1;
	}
	SH_GAME_API void TextureAsset::SetAssetData() const
	{
		if (!texturePtr.IsValid())
			return;

		TextureHeader header{};
		SetHeader(header);
		
		const auto& originalPixelData = texturePtr->GetPixelData();

		std::size_t pixelSize = 0;
		for (const auto& pixels : originalPixelData)
			pixelSize += pixels.size();

		std::vector<uint8_t> rawData(sizeof(TextureHeader) + pixelSize);

		std::memcpy(rawData.data(), &header, sizeof(TextureHeader));
		std::size_t offset = sizeof(TextureHeader);

		for (const auto& row : originalPixelData)
		{
			std::memcpy(rawData.data() + offset, row.data(), row.size());
			offset += row.size();
		}

		data = std::move(rawData);
	}

	SH_GAME_API auto TextureAsset::ParseAssetData() -> bool
	{
		texturePtr.Reset();

		if (data.size() < sizeof(TextureHeader))
			return false;

		std::memcpy(&header, data.data(), sizeof(TextureHeader));
		return true;
	}

	TextureAsset::TextureAsset() : 
		Asset(ASSET_NAME),
		texturePtr(nullptr)
	{
	}

	TextureAsset::TextureAsset(const render::Texture& texture) :
		Asset(ASSET_NAME),
		texturePtr(&texture)
	{
		assetUUID = texturePtr->GetUUID();
		SetHeader(header);
	}
	TextureAsset::~TextureAsset()
	{
	}
	SH_GAME_API void TextureAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Texture::GetStaticType())
			return;
		texturePtr = static_cast<const render::Texture*>(&obj);
		assetUUID = texturePtr->GetUUID();
		SetHeader(header);
	}
	SH_GAME_API auto TextureAsset::GetHeader() const -> TextureHeader
	{
		return header;
	}
	SH_GAME_API auto TextureAsset::GetPixelData() const -> PixelData
	{
		PixelData pixelData;
		pixelData.pixelDataPtr = data.data() + sizeof(TextureHeader);
		pixelData.size = data.size() - sizeof(TextureHeader);
		return pixelData;
	}
}//namespace