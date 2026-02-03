#include "Asset/FontGenerator.h"

#include "Core/Logger.h"
#include "Core/Util.h"

#include "Render/Texture.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "External/stb/stb_truetype.h"
namespace sh::game
{
    SH_GAME_API auto FontGenerator::GenerateFont(
        const render::IRenderContext& ctx,
        const std::vector<uint8_t>& fontData,
        std::string_view u8str,
        Options opt) -> render::Font*
    {
        std::vector<uint32_t> unicodes = ExtractUnicode(u8str);
        return GenerateFont(ctx, fontData, unicodes, opt);
    }

    SH_GAME_API auto FontGenerator::GenerateFont(const render::IRenderContext& ctx, const std::vector<uint8_t>& fontData, const std::vector<uint32_t> unicodes, Options opt) -> render::Font*
    {
        if (fontData.empty())
            return {};

        stbtt_fontinfo info{};
        int offset = stbtt_GetFontOffsetForIndex(fontData.data(), 0);
        if (!stbtt_InitFont(&info, fontData.data(), offset))
            return {};

        std::unordered_map<uint32_t, render::Font::Glyph> glyphs;
        const float scale = stbtt_ScaleForPixelHeight(&info, opt.fontSize);

        int ascent;
        int descent;
        int lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

        std::vector<AtlasPage> pages;
        pages.push_back(NewPage(opt));

        auto allocInPageFn =
            [&](int w, int h, int& outX, int& outY, uint32_t& outPage) -> bool
            {
                // packer에 할당 요청을 보내서 패킹 좌표를 얻어옴
                // 꽉찼고 옵션에서 멀티 페이지가 true인 경우 새 페이지를 만들어서 packer에 요청을 보냄
                for (int i = 0; i < pages.size(); ++i)
                {
                    if (pages[i].packer.Alloc(w, h, outX, outY))
                    {
                        outPage = i;
                        return true;
                    }
                }

                if (!opt.bAllowMultiPage)
                    return false;

                pages.push_back(NewPage(opt));
                const std::size_t last = pages.size() - 1;
                if (pages[last].packer.Alloc(w, h, outX, outY))
                {
                    outPage = last;
                    return true;
                }
                return false;
            };

        auto createGlyphFn =
            [&](uint32_t unicode) -> bool
            {
                auto it = glyphs.find(unicode);
                if (it != glyphs.end())
                    return true;

                int x0, y0, x1, y1;
                stbtt_GetCodepointBitmapBox(&info, (int)unicode, scale, scale, &x0, &y0, &x1, &y1);
                const int gw = x1 - x0;
                const int gh = y1 - y0;

                // advance = 원점에서부터의 실제 폰트가 차지하는 가로 길이
                int advance = 0;
                // lsb = 원점으로부터 폰트 시작위치의 가로 길이
                int lsb = 0;
                stbtt_GetCodepointHMetrics(&info, (int)unicode, &advance, &lsb);

                render::Font::Glyph g{};
                g.unicode = unicode;
                g.w = gw;
                g.h = gh;
                g.bearingX = x0;
                g.bearingY = y0;
                g.advance = (int)std::lround(advance * scale);

                // 공백 같은 경우는 atlas 할당 없이 advance만
                if (gw <= 0 || gh <= 0)
                {
                    glyphs.emplace(unicode, g);
                    return true;
                }

                int bitmapW = 0, bitmapH = 0, xoff = 0, yoff = 0;
                unsigned char* bmpData = stbtt_GetCodepointBitmap(&info, scale, scale, (int)unicode, &bitmapW, &bitmapH, &xoff, &yoff);

                // 보통 bw==gw, bh==gh
                uint32_t pageIndex = 0;
                int packX = 0, packY = 0;

                if (!allocInPageFn(bitmapW, bitmapH, packX, packY, pageIndex))
                    return false;

                AtlasPage& page = pages[pageIndex];

                for (int row = 0; row < bitmapH; ++row)
                {
                    uint8_t* dst = page.pixels.data() + (packY + row) * page.width + packX;
                    uint8_t* src = bmpData + row * bitmapW;
                    std::memcpy(dst, src, bitmapW);
                }
                stbtt_FreeBitmap(bmpData, nullptr);

                // uv
                g.page = pageIndex;
                g.u0 = (float)packX / (float)page.width;
                g.v0 = (float)packY / (float)page.height;
                g.u1 = (float)(packX + bitmapW) / (float)page.width;
                g.v1 = (float)(packY + bitmapH) / (float)page.height;

                glyphs.emplace(unicode, g);
                return true;
            };

        for (uint32_t code : unicodes)
            createGlyphFn(code);

        std::vector<render::Texture*> results;
        results.reserve(pages.size());
        for (auto& page : pages)
        {
            render::Texture* tex = core::SObject::Create<render::Texture>(render::TextureFormat::R8, page.width, page.height, false);
            tex->SetPixelData(std::move(page.pixels));
            tex->Build(ctx);

            results.push_back(tex);
        }

        render::Font::CreateInfo fontCi{};
        fontCi.atlases = std::move(results);
        fontCi.glyphs = std::move(glyphs);
        fontCi.ascent = ascent;
        fontCi.descent = descent;
        fontCi.lineGap = lineGap;
        fontCi.scale = scale;

        return core::SObject::Create<render::Font>(std::move(fontCi));
    }

    SH_GAME_API auto FontGenerator::ExtractUnicode(std::string_view u8str) -> std::vector<uint32_t>
    {
        std::vector<uint32_t> unicodes;

        std::set<uint32_t> codeSet;
        const char* start = u8str.data();
        const char* end = u8str.data() + u8str.size();
        while (start != end)
        {
            uint32_t code;
            start = core::Util::UTF8ToUnicode(start, end, code);
            codeSet.insert(code);
        }

        unicodes.reserve(codeSet.size());
        for (uint32_t code : codeSet)
            unicodes.push_back(code);

        return unicodes;
    }

    auto FontGenerator::NewPage(const Options& opt) -> AtlasPage
	{
		AtlasPage page{ render::ShelfPacker{ opt.atlasW, opt.atlasH, opt.padding } };
		page.width = opt.atlasW;
		page.height = opt.atlasH;
		page.pixels.assign((size_t)page.width * (size_t)page.height, 0);

        return page;
	}
}//namespace