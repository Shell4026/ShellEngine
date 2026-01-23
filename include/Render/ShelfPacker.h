#pragma once
#include "Export.h"

namespace sh::render
{
    class ShelfPacker
    {
    public:
        SH_RENDER_API ShelfPacker(int texW, int texH, int padding);

        SH_RENDER_API void Reset();

        SH_RENDER_API auto Alloc(int w, int h, int& outX, int& outY) -> bool;
    private:
        int texW = 0, texH = 0;
        int padding = 1;
        int cursorX = 0, cursorY = 0;
        int shelfH = 0;
    };
}//namespace