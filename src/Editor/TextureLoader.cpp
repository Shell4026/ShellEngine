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

		//int mip = glm::log2

		render::Texture* texture = core::SObject::Create<render::Texture>(format, width, height);
		texture->SetPixelData(pixels);
		stbi_image_free(pixels);

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
		mainJson["bSRGB"] = bSRGB;
		mainJson["bGenerateMipmap"] = bGenerateMipmap;
		return mainJson;
	}
	SH_EDITOR_API void TextureImporter::Deserialize(const core::Json& json)
	{
		if (json.contains("bSRGB"))
			bSRGB = json["bSRGB"];
		if (json.contains("bGenerateMipmap"))
			bGenerateMipmap = json["bGenerateMipmap"];
	}
}