#pragma once

#include "../include/Core/SContainer.hpp"
#include "../include/Core/SObject.h"

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <queue>
#include <map>

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
