#define STB_IMAGE_IMPLEMENTATION
#include "TextureLoader.h"

#include "Core/SObject.h"

#include <glm/ext.hpp>

namespace sh::editor
{
	TextureLoader::TextureLoader(const render::IRenderContext& context) :
		context(context)
	{
	}

	SH_EDITOR_API auto TextureLoader::Load(std::string_view filename, bool bGenerateMipmap) -> render::Texture*
	{
		int width, height, channel;
		int info = stbi_info(filename.data(), &width, &height, &channel);
		if (!info)
			return nullptr;

		stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);

		render::Texture::TextureFormat format;
		if (channel == 3)
			format = render::Texture::TextureFormat::SRGB24;

		//int mip = glm::log2

		render::Texture* texture = core::SObject::Create<render::Texture>(format, width, height);
		texture->SetPixelData(pixels);
		stbi_image_free(pixels);

		texture->Build(context);

		return texture;
	}
}