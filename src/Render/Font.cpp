#include "Font.h"

namespace sh::render
{
	Font::Font(const CreateInfo& ci) :
		atlases(ci.atlases),
		glyphs(ci.glyphs),
		ascent(ci.ascent), descent(ci.descent), scale(ci.scale), lineGap(ci.lineGap)
	{
	}
	Font::Font(CreateInfo&& ci) noexcept :
		atlases(std::move(ci.atlases)),
		glyphs(std::move(ci.glyphs)),
		ascent(ci.ascent), descent(ci.descent), scale(ci.scale), lineGap(ci.lineGap)
	{
	}
	Font::Font(Font&& other) noexcept :
		atlases(std::move(other.atlases)),
		glyphs(std::move(other.glyphs)),
		ascent(other.ascent), descent(other.descent), scale(other.scale), lineGap(other.lineGap)
	{
	}

	SH_RENDER_API auto Font::operator=(Font&& other) noexcept -> Font&
	{
		if (this == &other)
			return *this;

		atlases = std::move(other.atlases);
		glyphs = std::move(other.glyphs);

		ascent = (other.ascent);
		descent = (other.descent);
		scale = (other.scale);
		lineGap = (other.lineGap);

		return *this;
	}

	SH_RENDER_API auto Font::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json& fontJson = mainJson["Font"];
		fontJson["ascent"] = ascent;
		fontJson["descent"] = descent;
		fontJson["lineGap"] = lineGap;
		fontJson["scale"] = scale;

		core::Json& glyphJsons = mainJson["glyphs"];
		for (auto& [unicode, glyph] : glyphs)
		{
			core::Json glyphJson;
			glyphJson["unicode"] = unicode;
			glyphJson["w"] = glyph.w;
			glyphJson["h"] = glyph.h;
			glyphJson["bearingX"] = glyph.bearingX;
			glyphJson["bearingY"] = glyph.bearingY;
			glyphJson["advance"] = glyph.advance;
			glyphJson["page"] = glyph.page;
			glyphJson["u0"] = glyph.u0;
			glyphJson["v0"] = glyph.v0;
			glyphJson["u1"] = glyph.u1;
			glyphJson["v1"] = glyph.v1;

			glyphJsons.push_back(std::move(glyphJson));
		}
		return mainJson;
	}
	SH_RENDER_API void Font::Deserialize(const core::Json& json)
	{
		Clear();

		Super::Deserialize(json);
		const core::Json& fontJson = json["Font"];

		ascent = fontJson.value("ascent", 0.f);
		descent = fontJson.value("descent", 0.f);
		lineGap = fontJson.value("lineGap", 0.f);
		scale = fontJson.value("scale", 1.f);

		if (!json.contains("glyphs"))
			return;

		const core::Json& glyphJsons = json["glyphs"];
		for (const auto& glyphJson : glyphJsons)
		{
			Glyph glyph{};
			glyph.unicode = glyphJson["unicode"];
			glyph.w = glyphJson["w"];
			glyph.h = glyphJson["h"];
			glyph.bearingX = glyphJson["bearingX"];
			glyph.bearingY = glyphJson["bearingY"];
			glyph.advance = glyphJson["advance"];
			glyph.page = glyphJson["page"];
			glyph.u0 = glyphJson["u0"];
			glyph.v0 = glyphJson["v0"];
			glyph.u1 = glyphJson["u1"];
			glyph.v1 = glyphJson["v1"];

			glyphs.emplace(glyph.unicode, glyph);
		}
	}

	SH_RENDER_API void Font::Clear()
	{
		atlases.clear();
		glyphs.clear();
		ascent = descent = lineGap = 0.f;
		scale = 1.f;
	}

	SH_RENDER_API auto Font::GetGlyph(uint32_t unicode) const -> const Glyph*
	{
		auto it = glyphs.find(unicode);
		if (it == glyphs.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto Font::GetLineHeightPx() const -> float
	{
		return (ascent - descent + lineGap) * scale;
	}
}//namespace