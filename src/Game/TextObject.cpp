#include "TextObject.h"

namespace sh::game
{
    SH_GAME_API auto TextObject::operator=(const TextObject& other) -> TextObject&
    {
        text = other.text;

        return *this;
    }
    SH_GAME_API auto TextObject::operator=(TextObject&& other) noexcept -> TextObject&
    {
        text = std::move(other.text);

        return *this;
    }
}//namespace