﻿#pragma once

#include "../include/Core/Memory/MemoryPool.hpp"

#include <gtest/gtest.h>

#include <thread>

TEST(AllocateTest, MemoryPoolTest)
{
	class TestClass
	{
	public:
		int num;
		int num2;
	public:
		TestClass(int num) : num(num)
		{
			num2 = 123;
		}

		~TestClass() = default;
	};

	EXPECT_EQ(sizeof(TestClass), 8);
	sh::core::memory::MemoryPool<TestClass, 2> pool;
	TestClass* ptr1 = pool.Allocate();
	new (ptr1)TestClass{4};

	EXPECT_EQ(ptr1->num, 4);
	EXPECT_EQ(ptr1->num2, 123);
	EXPECT_EQ(pool.GetFreeSize(), 1);

	TestClass* ptr2 = pool.Allocate();
	new (ptr2)TestClass{ 5 };
	EXPECT_EQ(ptr2->num, 5);
	EXPECT_EQ(ptr2->num2, 123);
	EXPECT_EQ(pool.GetFreeSize(), 0);

	TestClass* ptr3 = pool.Allocate();
	new (ptr3)TestClass{ 6 };
	EXPECT_EQ(ptr3->num, 6);
	EXPECT_EQ(ptr3->num2, 123);
	EXPECT_EQ(pool.GetFreeSize(), 1);
	
	EXPECT_FALSE(pool.HasFreeBlock());

	ptr3->~TestClass();
	ptr2->~TestClass();
	ptr1->~TestClass();
	pool.DeAllocate(ptr3);
	pool.DeAllocate(ptr2);
	pool.DeAllocate(ptr1);

	EXPECT_TRUE(pool.HasFreeBlock());

	ptr1 = pool.Allocate();
	EXPECT_EQ(pool.GetFreeSize(), 1);
}

TEST(AllocateTest, MemoryPoolThreadTest)
{
	sh::core::memory::MemoryPool<int, 10> pool{};

	auto func1 = [&]
	{
		for (int i = 0; i < 5; ++i)
		{
			int* ptr = pool.Allocate();
			*ptr = i * 100;
			pool.DeAllocate(ptr);
		}
	};
	auto func2 = [&]
	{
		for (int i = 0; i < 5; ++i)
		{
			int* ptr = pool.Allocate();
			*ptr = i;
		}
	};

	std::thread thr1{ func1 };
	std::thread thr2{ func2 };

	thr1.join();
	thr2.join();

	EXPECT_EQ(pool.GetFreeSize(), 5);
}