#pragma once
#include "Export.h"

#include "Core/SObject.h"

#include "Render/Texture.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
namespace sh::render
{
	class Font : public core::SObject
	{
		SCLASS(Font)
	public:
		struct Glyph
		{
			uint32_t unicode = 0;

			int w = 0, h = 0;

			int bearingX = 0;
			int bearingY = 0;

			int advance = 0;

			uint32_t page = 0;
			float u0 = 0, v0 = 0;
			float u1 = 0, v1 = 0;
		};
		struct CreateInfo
		{
			std::vector<uint8_t> fontData;
			std::vector<render::Texture*> atlases;
			std::unordered_map<uint32_t, Glyph> glyphs;
			int ascent = 0;
			int descent = 0;
			int lineGap = 0;
			float scale = 1.0f;
		};
	public:
		SH_RENDER_API Font(const CreateInfo& ci);
		SH_RENDER_API Font(CreateInfo&& ci) noexcept;
		SH_RENDER_API Font(Font&& other) noexcept;

		SH_RENDER_API auto operator=(Font&& other) noexcept -> Font&;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;

		SH_RENDER_API void Clear();

		SH_RENDER_API auto GetGlyph(uint32_t unicode) const -> const Glyph*;
		SH_RENDER_API auto GetLineHeightPx() const -> float;
		SH_RENDER_API auto GetAtlases() const -> const std::vector<render::Texture*>& { return atlases; }
		SH_RENDER_API auto GetFontData() const -> std::vector<uint8_t> { return fontData; }
		SH_RENDER_API auto GetScale() const -> float { return scale; }
	private:
		std::vector<uint8_t> fontData;
		PROPERTY(atlases)
		std::vector<render::Texture*> atlases;
		std::unordered_map<uint32_t, Glyph> glyphs;
		int ascent = 0;
		int descent = 0;
		int lineGap = 0;
		float scale = 1.0f;
	};
}//namespace