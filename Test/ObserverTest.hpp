#pragma once

#include "../include/Core/Observer.h"

#include <gtest/gtest.h>

class Target
{
private:
	sh::core::Observer* observer;
	int number;
public:
	Target(sh::core::Observer* observer) :
		observer(observer), number(0)
	{

	}

	void ChangeValue(int num)
	{
		number = num;
		observer->Notify();
	}

	auto GetNumber() const -> int
	{
		return number;
	}
};

TEST(ObserverTest, NotifyTest) 
{
	int value = 0;

	sh::core::Observer observer{};
	Target target{ &observer };

	sh::core::Observer::EventHandle handle1 =
	observer.RegisterEvent([&] 
	{
		value = target.GetNumber() - 1;
	}, -1);
	sh::core::Observer::EventHandle handle2 = 
	observer.RegisterEvent([&] 
	{
		value = target.GetNumber();
	}, 2);
	sh::core::Observer::EventHandle handle3 =
	observer.RegisterEvent([&] 
	{
		value = target.GetNumber() + 1;
	}, 1);

	target.ChangeValue(1);
	EXPECT_EQ(value, 1);

	observer.RemoveEvent(handle2);
	target.ChangeValue(2);
	EXPECT_EQ(value, 3);

	observer.RemoveEvent(handle3);
	target.ChangeValue(3);
	EXPECT_EQ(value, 2);

	observer.RemoveEvent(handle1);
	target.ChangeValue(4);
	EXPECT_EQ(value, 2);
}