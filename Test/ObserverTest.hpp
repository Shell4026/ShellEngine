#pragma once

#include "../include/Core/Observer.hpp"

#include <gtest/gtest.h>

class Target
{
private:
	int number;
public:
	sh::core::Observer<false, int> observer{};
public:
	Target() :
		 number(0)
	{

	}

	void ChangeValue(int num)
	{
		number = num;
		observer.Notify(num);
	}

	auto GetNumber() const -> int
	{
		return number;
	}
};

TEST(ObserverTest, NotifyTesting)
{
	int value = 0;

	Target target{};

	sh::core::Observer<false, int>::Listener listener
	{
		[&](int num)
		{
			value = num + 1;
		}
	};
	target.observer.Register(listener);

	target.ChangeValue(1);

	EXPECT_EQ(value, 2);
}

TEST(ObserverTest, PriorityTesting) 
{
	int value = 0;

	Target target{};

	sh::core::Observer<false, int>::Listener listener1
	{ 
		[&](int num)
		{
			value = num - 1;
		},
		0
	};
	sh::core::Observer<false, int>::Listener listener2
	{
		[&](int num)
		{
			value = num;
		},
		1
	};
	sh::core::Observer<false, int>::Listener listener3
	{
		[&](int num)
		{
			value = num + 1;
		},
		2
	};
	target.observer.Register(listener1);
	target.observer.Register(listener2);
	target.observer.Register(listener3);

	target.ChangeValue(1);
	EXPECT_EQ(value, 0); // handle3->2->1

	target.observer.UnRegister(listener1);
	target.ChangeValue(2);
	EXPECT_EQ(value, 2); // handle3->2

	target.observer.UnRegister(listener2);
	target.ChangeValue(3);
	EXPECT_EQ(value, 4); // handle3

	target.observer.UnRegister(listener3);
	target.ChangeValue(4);
	EXPECT_EQ(value, 4); // 등록된 이벤트가 없으므로 이전과 같은 값
}

TEST(ObserverTest, AutoUnRegisterTesting)
{
	int value = 0;

	Target target{};

	{
		sh::core::Observer<false, int>::Listener listener
		{
			[&](int num)
			{
				value = num - 1;
			}
		};
		target.observer.Register(listener);
		target.ChangeValue(5);
		EXPECT_EQ(value, 4);
	}//listener는 자동으로 옵저버에서 UnRegister

	value = 123;
	target.ChangeValue(5);
	EXPECT_EQ(value, 123);
}