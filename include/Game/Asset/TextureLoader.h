#pragma once
#include "../Export.h"
#include "TextureAsset.h"

#include "Core/UUID.h"
#include "Core/IAssetLoader.h"

#include "Render/Texture.h"

#include <filesystem>
#include <vector>
namespace sh::render
{
	class IRenderContext;
}
namespace sh::game
{
	class TextureLoader : public core::IAssetLoader
	{
	private:
		static constexpr const char* ASSET_NAME = "tex";
	public:
		const render::IRenderContext& context;
	private:
		auto GenerateMipmaps(uint8_t* pixels, uint32_t width, uint32_t height) -> std::vector<std::vector<uint8_t>>;
	public:
		SH_GAME_API TextureLoader(const render::IRenderContext& context);
		SH_GAME_API ~TextureLoader() = default;
		SH_GAME_API auto Load(const std::filesystem::path& filePath) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;

		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}