#pragma once
#include "Export.h"
#include "Reflection/TypeTraits.hpp"

namespace sh::core
{
    class IEvent
    {
    public:
        virtual ~IEvent() = default;
        virtual auto GetTypeHash() const -> std::size_t = 0;
    };
}