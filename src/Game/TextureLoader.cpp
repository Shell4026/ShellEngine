#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "TextureLoader.h"

#include "Core/SObject.h"

#include "External/stb/stb_image.h"
#include "External/stb/stb_image_resize2.h"

#include <glm/ext.hpp>

namespace sh::game
{
	auto TextureLoader::GenerateMipmaps(uint8_t* pixels, uint32_t width, uint32_t height) -> std::vector<std::vector<uint8_t>>
	{
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
		uint32_t mipWidth = width;
		uint32_t mipHeight = height;

		std::vector<std::vector<uint8_t>> mipmaps(mipLevels);
		mipmaps[0].resize(width * height * 4);
		std::memcpy(mipmaps[0].data(), pixels, width * height * 4);

		for (int i = 1; i < mipLevels; ++i) 
		{
			int prevWidth = mipWidth;
			int prevHeight = mipHeight;
			mipWidth = std::max(1, prevWidth / 2);
			mipHeight = std::max(1, prevHeight / 2);

			mipmaps[i].resize(mipWidth * mipHeight * 4);

			stbir_resize_uint8_linear(
				mipmaps[i - 1].data(), prevWidth, prevHeight, 0,
				mipmaps[i].data(), mipWidth, mipHeight, 0, STBIR_4CHANNEL
			);
		}
		return mipmaps;
	}
	TextureLoader::TextureLoader(const render::IRenderContext& context) :
		context(context)
	{
	}
	SH_GAME_API auto TextureLoader::Load(const std::filesystem::path& filePath) -> core::SObject*
	{
		int width, height, channel;
		int info = stbi_info(filePath.string().c_str(), &width, &height, &channel);
		if (!info)
			return nullptr;

		stbi_uc* pixels = stbi_load(filePath.string().c_str(), &width, &height, &channel, STBI_rgb_alpha);

		render::Texture::TextureFormat format = render::Texture::TextureFormat::SRGBA32;
		if (channel == 3)
		{
			format = render::Texture::TextureFormat::SRGB24;
		}
		
		auto mipMaps = GenerateMipmaps(pixels, width, height);

		render::Texture* texture = core::SObject::Create<render::Texture>(format, width, height, true);
		for (int m = 0; m < mipMaps.size(); ++m)
			texture->SetPixelData(mipMaps[m], m);
		stbi_image_free(pixels);

		texture->Build(context);

		return texture;
	}
	SH_GAME_API auto TextureLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a texture!", asset.GetAssetUUID().ToString());
			return nullptr;
		}
		const auto& texAsset = static_cast<const game::TextureAsset&>(asset);
		const auto header = texAsset.GetHeader();
		const auto pixelData = texAsset.GetPixelData();

		render::Texture* texture = core::SObject::Create<render::Texture>(header.format, header.width, header.height, header.bMipmap);
		texture->SetUUID(asset.GetAssetUUID());

		std::size_t offset = 0;
		for (uint32_t mip = 0; mip < texture->GetMipLevel(); ++mip)
		{
			std::size_t size = texture->GetPixelData(mip).size();
			texture->SetPixelData(pixelData.pixelDataPtr + offset, size, mip);
			offset += size;
		}

		texture->SetAnisoLevel(header.aniso);
		texture->Build(context);

		return texture;
	}
	SH_GAME_API auto TextureLoader::GetAssetName() const -> const char*
	{
		return ASSET_NAME;
	}
}