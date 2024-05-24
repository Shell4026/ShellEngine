#define STB_IMAGE_IMPLEMENTATION

#include "TextureLoader.h"

namespace sh::game
{
	auto TextureLoader::Load(std::string_view filename) -> std::unique_ptr<render::Texture>
	{
		int width, height, channel;
		int info = stbi_info(filename.data(), &width, &height, &channel);
		if (!info)
			return nullptr;

		stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);

		render::Texture::TextureFormat format;
		if (channel == 3)
			format = render::Texture::TextureFormat::RGB24;

		std::unique_ptr<render::Texture> texture = std::make_unique<render::Texture>(format, width, height);
		texture->SetPixelData(pixels);
		stbi_image_free(pixels);

		return texture;
	}
}