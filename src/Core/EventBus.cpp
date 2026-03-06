#include "EventBus.h"
#include "IEvent.h"
#include "EventSubscriber.h"
#include <algorithm>

namespace sh::core
{
    EventBus::~EventBus()
    {
        for (auto& [hash, subscribers] : listeners)
        {
            for (auto& subscriber : subscribers)
            {
                subscriber->eventBus = nullptr;
            }
        }
    }

    SH_CORE_API void EventBus::Subscribe(ISubscriber& subscriber)
    {
        subscriber.eventBus = this;
        const auto eventTypeHash = subscriber.GetEventTypeHash();
        listeners[eventTypeHash].push_back(&subscriber);
    }
    SH_CORE_API void EventBus::Unsubscribe(ISubscriber& subscriber)
    {
        const auto eventTypeHash = subscriber.GetEventTypeHash();
        auto it = listeners.find(eventTypeHash);
        if (it == listeners.end())
            return;
        auto& subscribers = it->second;
        auto removeIt = std::remove(subscribers.begin(), subscribers.end(), &subscriber);
        if (removeIt == subscribers.end())
            return;

        subscriber.eventBus = nullptr;
        subscribers.erase(removeIt, subscribers.end());
    }
    SH_CORE_API void EventBus::Publish(const IEvent& event)
    {
        const auto eventTypeHash = event.GetTypeHash();
        auto it = listeners.find(eventTypeHash);
        if (it == listeners.end())
            return;

        std::vector<ISubscriber*> subscribersCopy = it->second;
        for (auto* subscriber : subscribersCopy)
        {
            if (subscriber->eventBus)
                subscriber->Invoke(event);
        }
    }
}