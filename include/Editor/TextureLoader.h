﻿#pragma once

#include "Export.h"
#include "IImporter.h"

#include "Render/Texture.h"

#include "External/stb/stb_image.h"

#include <string_view>

namespace sh::render
{
	class IRenderContext;
}
namespace sh::editor
{
	class TextureImporter : public IImporter
	{
		friend class TextureLoader;
	private:
		const char* name = "TextureImporter";
	public:
		bool bSRGB = false;
		bool bGenerateMipmap = true;
	public:
		SH_EDITOR_API auto GetName() const -> const char* override;
		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};

	class TextureLoader
	{
	public:
		const render::IRenderContext& context;
	public:
		SH_EDITOR_API TextureLoader(const render::IRenderContext& context);
		SH_EDITOR_API ~TextureLoader() = default;
		SH_EDITOR_API auto Load(std::string_view filename) -> render::Texture*;
		SH_EDITOR_API auto Load(std::string_view filename, const TextureImporter& option) -> render::Texture*;
	};
}