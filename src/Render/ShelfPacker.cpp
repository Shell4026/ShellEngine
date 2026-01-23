#include "ShelfPacker.h"

namespace sh::render
{
    ShelfPacker::ShelfPacker(int texW, int texH, int padding)
    {
        this->texW = texW; this->texH = texH;
        this->padding = padding;
        cursorX = 0;
        cursorY = 0;
        shelfH = 0;
    }
    SH_RENDER_API void ShelfPacker::Reset()
    {
        cursorX = 0;
        cursorY = 0;
        shelfH = 0;
    }
    SH_RENDER_API auto ShelfPacker::Alloc(int w, int h, int& outX, int& outY) -> bool
    {
        w += padding * 2;
        h += padding * 2;

        if (w > texW || h > texH) 
            return false;

        if (cursorX + w > texW)
        {
            cursorX = 0;
            cursorY += shelfH;
            shelfH = 0;
        }

        if (cursorY + h > texH) 
            return false;

        outX = cursorX + padding;
        outY = cursorY + padding;

        cursorX += w;
        if (h > shelfH) 
            shelfH = h;

        return true;
    }
}//namespace