﻿#pragma once

#include "../include/Core/SContainer.hpp"
#include "../include/Core/SObject.h"

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <queue>
#include <map>
#include <memory>

TEST(ContainerTest, DestructorTest)
{
	using namespace sh;
	class MoveClass
	{
	private:
		std::unique_ptr<int> ptr;
	public:
		MoveClass(int num) :
			ptr(new int(num))
		{}
		MoveClass(const MoveClass& other) = delete;
		MoveClass(MoveClass&& other) noexcept :
			ptr(std::move(other.ptr))
		{}
	};


	core::SMap<int, MoveClass, 8> map0{};
	core::SMap<int, MoveClass, 8> map1{};

	for (int i = 0; i < 10; ++i)
	{
		map0.insert({ i, MoveClass{i} });
	}
	map1 = std::move(map0);
}

TEST(ContainerTest, SSetTest)
{
	class TestClass : public sh::core::SObject
	{
	public:
		int num = 123;
		TestClass() = default;
	};

	using namespace sh::core;

	SSet<int, 128> container1;
	std::set<int> container2;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		container1.insert(i);
		if (i % 2 == 1)
			container1.erase(i - 1);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto time1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		container2.insert(i);
		if (i % 2 == 1)
			container2.erase(i - 1);
	}
	end = std::chrono::high_resolution_clock::now();
	auto time2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	EXPECT_LE(time1, time2); // SSet이 평균적으로 메모리 풀을 쓰기 때문에 기본보다 더 빠르다.

	SSet<int, 128> container3{};
	auto alloc = container3.get_allocator();
	container3 = std::move(container1);
	EXPECT_EQ(container3.size(), 5000);
}

TEST(ContainerTest, LockFreeQueue)
{
	sh::core::LockFreeQueue<int> lockFreeQueue;
	std::queue<int> queue;

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		queue.push(i);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto timeQueue = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; ++i)
	{
		lockFreeQueue.Enqueue(i);
	}
	end = std::chrono::high_resolution_clock::now();
	auto timeQueue2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	EXPECT_LE(timeQueue, timeQueue2);
}

TEST(ContainerTest, SHashMapVectorTest)
{
	using namespace sh;
	core::SHashMapVector<std::string, int> hvec;
	hvec.Insert("test1", 1);
	hvec.Insert("test2", 2);
	hvec.Insert("test3", 3);

	int i = 1;
	for (auto& v : hvec)
	{
		if (v)
		{
			EXPECT_EQ(*v, i++);
		}
	}
	hvec.Erase("test2");
	EXPECT_FALSE(hvec[1].has_value());
	EXPECT_EQ(hvec.Size(), 2);
	EXPECT_EQ(hvec.AllocatedSize(), 3);
	auto it = hvec.Find("Test2");
	EXPECT_TRUE(it == hvec.end());
	it = hvec.Find("test3");
	EXPECT_FALSE(it == hvec.end());
	EXPECT_EQ(it->value(), 3);

	for (int i = 0; i < 7; ++i)
	{
		hvec.Insert("test" + std::to_string(4 + i), 4 + i);
	}
	EXPECT_EQ(hvec.Size(), 9);
	EXPECT_EQ(hvec.AllocatedSize(), 9);

	hvec.Clear();
	EXPECT_EQ(hvec.Size(), 0);
	EXPECT_EQ(hvec.AllocatedSize(), 0);

	for (int i = 0; i < 10; ++i)
	{
		hvec.Insert("test" + std::to_string(i), i);
	}
	for (int i = 0; i < 8; ++i)
	{
		hvec.Erase("test" + std::to_string(i));
	}
	EXPECT_EQ(hvec.Size(), 2);
	EXPECT_EQ(hvec.AllocatedSize(), 2);
}