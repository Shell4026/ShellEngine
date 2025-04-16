#pragma once
#include "Core/ThreadPool.h"

#include <gtest/gtest.h>

TEST(ThreadPoolTest, ThreadPoolTest)
{
	using namespace sh;

	auto threadPool = core::ThreadPool::GetInstance();
	threadPool->Init(4);
	int num = 0;
	auto fn = 
		[&]() -> int
		{
			for (int i = 0; i < 100000; ++i)
				++num;
			return num;
		};
	auto future1 = threadPool->AddTask(fn);
	auto future2 = threadPool->AddTask(fn);
	int result1 = future1.get();
	int result2 = future2.get();
	EXPECT_NE(result1, result2); // 낮은 확률로 같을지도
}