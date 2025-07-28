#pragma once
#include <gtest/gtest.h>
#include "Core/IEvent.h"
#include "Core/EventBus.h"
#include "Core/EventSubscriber.h"

namespace sh::core
{
    // 테스트용 이벤트 1
    class TestEvent : public IEvent
    {
    public:
        int value = 0;
        TestEvent(int v) : 
            value(v)
        {}
        auto GetTypeHash() const -> std::size_t override 
        { 
            return reflection::TypeTraits::GetTypeHash<TestEvent>(); 
        }
    };

    // 테스트용 이벤트 2
    class AnotherTestEvent : public IEvent
    {
    public:
        std::string message;
        AnotherTestEvent(std::string m) : 
            message(m) 
        {}
        auto GetTypeHash() const -> std::size_t override 
        {
            return reflection::TypeTraits::GetTypeHash<AnotherTestEvent>(); 
        }
    };

    TEST(EventBusTest, SubscribeAndPublish)
    {
        EventBus bus;
        EventSubscriber<TestEvent> subscriber;

        int receivedValue = 0;
        subscriber.SetCallback(
            [&](const TestEvent& e) 
            {
                receivedValue = e.value;
            }
        );

        bus.Subscribe(subscriber);

        TestEvent event(42);
        bus.Publish(event);

        EXPECT_EQ(receivedValue, 42);
    }

    TEST(EventBusTest, MultipleSubscribers)
    {
        EventBus bus;
        EventSubscriber<TestEvent> subscriber1;
        EventSubscriber<TestEvent> subscriber2;

        int receivedValue1 = 0;
        subscriber1.SetCallback(
            [&](const TestEvent& e) 
            {
                receivedValue1 = e.value;
            }
        );

        int receivedValue2 = 0;
        subscriber2.SetCallback(
            [&](const TestEvent& e) 
            {
            receivedValue2 = e.value;
            }
        );

        bus.Subscribe(subscriber1);
        bus.Subscribe(subscriber2);

        TestEvent event(100);
        bus.Publish(event);

        EXPECT_EQ(receivedValue1, 100);
        EXPECT_EQ(receivedValue2, 100);
    }

    TEST(EventBusTest, Unsubscribe)
    {
        EventBus bus;
        EventSubscriber<TestEvent> subscriber;

        bool wasCalled = false;
        subscriber.SetCallback(
            [&](const TestEvent& e) 
            {
                wasCalled = true;
            }
        );

        bus.Subscribe(subscriber);
        bus.Unsubscribe(subscriber);

        TestEvent event(123);
        bus.Publish(event);

        EXPECT_FALSE(wasCalled);
    }

    TEST(EventBusTest, DifferentEvents)
    {
        EventBus bus;
        EventSubscriber<TestEvent> subscriber1;
        EventSubscriber<AnotherTestEvent> subscriber2;

        int receivedValue = 0;
        subscriber1.SetCallback(
            [&](const TestEvent& e) 
            {
                receivedValue = e.value;
            }
        );

        std::string receivedMessage;
        subscriber2.SetCallback(
            [&](const AnotherTestEvent& e) 
            {
                receivedMessage = e.message;
            }
        );

        bus.Subscribe(subscriber1);
        bus.Subscribe(subscriber2);

        TestEvent event1(99);
        bus.Publish(event1);

        AnotherTestEvent event2("hello");
        bus.Publish(event2);

        EXPECT_EQ(receivedValue, 99);
        EXPECT_EQ(receivedMessage, "hello");
    }

    TEST(EventBusTest, NoSubscribers)
    {
        EventBus bus;
        TestEvent event(1);
        // 아무 일도 일어나지 않고 크래시가 나지 않아야 함
        ASSERT_NO_THROW(bus.Publish(event));
    }
}
