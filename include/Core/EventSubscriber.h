#pragma once
#include "Export.h"
#include "IEvent.h"
#include "EventBus.h"
#include "Reflection/TypeTraits.hpp"

#include <functional>
namespace sh::core
{
    class ISubscriber
    {
    protected:
        friend class EventBus;
        EventBus* eventBus = nullptr;
    public:
        virtual ~ISubscriber() = default;
        virtual void Invoke(const IEvent& event) = 0;
        virtual auto GetEventTypeHash() const -> std::size_t = 0;
    };

    template<typename T>
    class EventSubscriber : public ISubscriber
    {
        static_assert(std::is_base_of_v<IEvent, T>, "T must be a descendant of Event");
    private:
        std::function<void(const T&)> callback;
    public:
        EventSubscriber() = default;
        ~EventSubscriber()
        {
            if (eventBus != nullptr)
                eventBus->Unsubscribe(*this);
        }

        EventSubscriber(const EventSubscriber&) = delete;
        auto operator=(const EventSubscriber&) -> EventSubscriber& = delete;

        void SetCallback(std::function<void(const T&)> callback)
        {
            this->callback = callback;
        }

        void Invoke(const IEvent& event) override
        {
            if (callback)
                callback(static_cast<const T&>(event));
        }

        auto GetEventTypeHash() const ->  std::size_t override
        {
            return reflection::TypeTraits::GetTypeHash<T>();
        }
    };
}