#pragma once
#include "../include/Core/SpinLock.h"

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>
TEST(SpinLockTest, LockTest)
{
	using namespace sh;
	core::SpinLock lock{};
	int num = 0;
	auto func = [&](int n)
		{
			lock.lock();
			for (int i = 0; i < n; ++i)
			{
				num += 1;
			}
			lock.unlock();
		};
	const int n = 4;
	const int r = 100'000;

	std::vector<std::thread> threads;
	for (int i = 0; i < n; ++i)
		threads.push_back(std::thread{ func, r });

	for (int i = 0; i < n; ++i)
		threads[i].join();

	EXPECT_EQ(num, r * n);
}
