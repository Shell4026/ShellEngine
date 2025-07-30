#pragma once
#include "Export.h"

#include <vector>
#include <unordered_map>
#include <cstddef>
namespace sh::core
{
    class IEvent;
    class ISubscriber;

    class EventBus
    {
    private:
        std::unordered_map<std::size_t, std::vector<ISubscriber*>> listeners;
    public:
        SH_CORE_API ~EventBus();

        SH_CORE_API void Subscribe(ISubscriber& subscriber);
        SH_CORE_API void Unsubscribe(ISubscriber& subscriber);
        SH_CORE_API void Publish(const IEvent& event);
    };
}