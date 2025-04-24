#pragma once

#include "../include/Core/SContainer.hpp"
#include "../include/Core/SObject.h"

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <queue>
#include <map>
#include <memory>

TEST(ContainerTest, Test)
{
	using namespace sh::core;

	SVector<int> arr{};
	for (auto n : arr)
	{
		std::cout << n;
	}
}