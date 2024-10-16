#pragma once

#include <Core/Singleton.hpp>

#include <gtest/gtest.h>

#include <thread>

class SingleClass : public sh::core::Singleton<SingleClass>
{
public:
	int num;
public:
	SingleClass() : num(0)
	{

	}
	~SingleClass()
	{

	}
};

class SingleClassWithArgs : public sh::core::Singleton<SingleClassWithArgs>
{
public:
	int num0, num1;
public:
	SingleClassWithArgs(int num0, int num1) : 
		num0(num0), num1(num1)
	{}
};

TEST(SingletonTest, GetInstancingTesting)
{
	SingleClass* instance = SingleClass::GetInstance();
	instance->num = 4;
	EXPECT_EQ(instance->num, 4);
	instance->Destroy();
}

TEST(SingletonTest, MultiThreadTesting)
{
	SingleClass* ptr1 = nullptr;
	SingleClass* ptr2 = nullptr;

	for (int i = 0; i < 10; ++i)
	{
		std::thread thr1
		{ [&]
			{
				ptr1 = SingleClass::GetInstance();
				ptr1->num = 1;
			}
		};
		std::thread thr2
		{ [&]
			{
				ptr2 = SingleClass::GetInstance();
				ptr2->num = 2;
			}
		};

		thr1.join();
		thr2.join();
		EXPECT_EQ(ptr1, ptr2);
	}
}

TEST(SingletonTest, ArgsTesting)
{
	SingleClassWithArgs* ptr = SingleClassWithArgs::GetInstance(1, 2);
	EXPECT_EQ(ptr->num0, 1);
	EXPECT_EQ(ptr->num1, 2);
	SingleClassWithArgs* ptr2 = SingleClassWithArgs::GetInstance(0, 0);
	EXPECT_EQ(ptr2->num0, 1);
	EXPECT_EQ(ptr2->num1, 2);
}
