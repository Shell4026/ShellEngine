#pragma once

namespace sh::phys
{
    using Tagbit = uint16_t;
    enum Tag
    {
        Tag1 = 0x0001,
        Tag2 = 0x0002,
        Tag3 = 0x0004,
        Tag4 = 0x0008,
        Tag5 = 0x0010,
        Tag6 = 0x0020,
        Tag7 = 0x0040,
        Tag8 = 0x0080,
        Tag9 = 0x0100,
        Tag10 = 0x0200,
        Tag11 = 0x0400,
        Tag12 = 0x0800,
        Tag13 = 0x1000,
        Tag14 = 0x2000,
        Tag15 = 0x4000,
        Tag16 = 0x8000
    };
}