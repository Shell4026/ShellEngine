#include "EventBus.h"
#include "IEvent.h"
#include "EventSubscriber.h"
#include <algorithm>

namespace sh::core
{
    SH_CORE_API void EventBus::Subscribe(ISubscriber& subscriber)
    {
        subscriber.eventBus = this;
        const auto eventTypeHash = subscriber.GetEventTypeHash();
        listeners[eventTypeHash].push_back(&subscriber);
    }
    SH_CORE_API void EventBus::Unsubscribe(ISubscriber& subscriber)
    {
        subscriber.eventBus = nullptr;
        const auto eventTypeHash = subscriber.GetEventTypeHash();
        if (listeners.count(eventTypeHash))
        {
            auto& subscribers = listeners.at(eventTypeHash);
            subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), &subscriber), subscribers.end());
        }
    }
    SH_CORE_API void EventBus::Publish(const IEvent& event)
    {
        const auto eventTypeHash = event.GetTypeHash();
        if (listeners.count(eventTypeHash))
        {
            auto subscribersCopy = listeners.at(eventTypeHash);
            for (auto* subscriber : subscribersCopy)
            {
                if (subscriber->eventBus) 
                    subscriber->Invoke(event);
            }
        }
    }
}