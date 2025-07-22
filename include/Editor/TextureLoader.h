#pragma once
#include "Export.h"

#include "Core/UUID.h"
#include "Core/IAssetLoader.h"

#include "Game/TextureAsset.h"

#include "Render/Texture.h"

#include <filesystem>
#include <vector>
namespace sh::render
{
	class IRenderContext;
}
namespace sh::editor
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
		SH_EDITOR_API TextureLoader(const render::IRenderContext& context);
		SH_EDITOR_API ~TextureLoader() = default;
		SH_EDITOR_API auto Load(const std::filesystem::path& filePath) -> core::SObject* override;
		SH_EDITOR_API auto Load(const core::Asset& asset) -> core::SObject* override;

		SH_EDITOR_API auto GetAssetName() const -> const char* override;
	};
}