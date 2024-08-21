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