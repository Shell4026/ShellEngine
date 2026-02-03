#pragma once
#include "Game/Export.h"

#include "Render/IRenderContext.h"
#include "Render/ShelfPacker.h"
#include "Render/Font.h"

#include <cstdint>
#include <vector>
#include <string_view>
namespace sh::game
{
    class FontGenerator
    {
    public:
        struct Options
        {
            int atlasW = 2048;
            int atlasH = 2048;
            int padding = 1;

            float fontSize = 32.f;

            bool bAllowMultiPage = true;
        };
        struct AtlasPage
        {
            render::ShelfPacker packer;
            int width = 0;
            int height = 0;
            std::vector<uint8_t> pixels;
        };
    public:
        SH_GAME_API static auto GenerateFont(
            const render::IRenderContext& ctx,
            const std::vector<uint8_t>& fontData,
            std::string_view u8str,
            Options opt
        ) -> render::Font*;
        SH_GAME_API static auto GenerateFont(
            const render::IRenderContext& ctx,
            const std::vector<uint8_t>& fontData,
            const std::vector<uint32_t> unicodes,
            Options opt
        ) -> render::Font*;

        SH_GAME_API static auto ExtractUnicode(std::string_view u8str) -> std::vector<uint32_t>;
    private:
        static auto NewPage(const Options& opt) -> AtlasPage;
    };
}//namespace