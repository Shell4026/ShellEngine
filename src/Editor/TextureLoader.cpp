#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "TextureLoader.h"

#include "Core/SObject.h"

#include "External/stb/stb_image.h"
#include "External/stb/stb_image_resize2.h"

#include <glm/ext.hpp>

namespace sh::editor
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

	SH_EDITOR_API auto TextureLoader::Load(std::string_view filename) -> render::Texture*
	{
		return Load(filename, TextureImporter{});
	}

	SH_EDITOR_API auto TextureLoader::Load(std::string_view filename, const TextureImporter& option) -> render::Texture*
	{
		int width, height, channel;
		int info = stbi_info(filename.data(), &width, &height, &channel);
		if (!info)
			return nullptr;

		stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);

		render::Texture::TextureFormat format;
		if (channel == 3)
		{
			format = option.bSRGB ? 
				render::Texture::TextureFormat::SRGB24 : render::Texture::TextureFormat::RGB24;
		}
		
		auto mipMaps = GenerateMipmaps(pixels, width, height);

		render::Texture* texture = core::SObject::Create<render::Texture>(format, width, height, true);
		for (int m = 0; m < mipMaps.size(); ++m)
			texture->SetPixelData(mipMaps[m], m);
		stbi_image_free(pixels);

		texture->SetAnisoLevel(option.aniso);
		texture->Build(context);

		return texture;
	}
	SH_EDITOR_API auto TextureImporter::GetName() const -> const char*
	{
		return name;
	}
	SH_EDITOR_API auto TextureImporter::Serialize() const -> core::Json
	{
		core::Json mainJson{};
		mainJson["version"] = 1;
		mainJson["aniso"] = aniso;
		mainJson["bSRGB"] = bSRGB;
		mainJson["bGenerateMipmap"] = bGenerateMipmap;
		return mainJson;
	}
	SH_EDITOR_API void TextureImporter::Deserialize(const core::Json& json)
	{
		if (json.contains("aniso"))
			aniso = json["aniso"];
		if (json.contains("bSRGB"))
			bSRGB = json["bSRGB"];
		if (json.contains("bGenerateMipmap"))
			bGenerateMipmap = json["bGenerateMipmap"];
	}
}