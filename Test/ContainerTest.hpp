#pragma once

#include "../include/Core/SContainer.hpp"
#include "../include/Core/SObject.h"
#include "../include/Core/GarbageCollection.h"

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <queue>
#include <map>
#include <memory>

TEST(ContainerTest, Test)
{
	using namespace sh::core;

	auto gc = GarbageCollection::GetInstance();

	SVector<int> arr{};
	for (auto n : arr)
	{
		std::cout << n;
	}

	SObject* objInContainer = SObject::Create<SObject>();
	SObject* objOutContainer = SObject::Create<SObject>();
	SHashMap<SObject*, int> hashMap0;
	SHashMap<int, SObject*> hashMap1;
	hashMap0[objInContainer] = 0;
	hashMap1[0] = objInContainer;
	EXPECT_EQ(gc->GetObjectCount(), 2);
	gc->Collect();
	EXPECT_TRUE(IsValid(objInContainer));
	EXPECT_FALSE(IsValid(objOutContainer));
	gc->DestroyPendingKillObjs();
	EXPECT_EQ(gc->GetObjectCount(), 1);
	EXPECT_EQ(hashMap0.size(), 1);
	EXPECT_EQ(hashMap1.size(), 1);
	objInContainer->Destroy();
	EXPECT_FALSE(IsValid(objInContainer));
	gc->Collect(); // 여기서 hashMap0,1의 원소가 제거됨
	EXPECT_EQ(hashMap0.size(), 0);
	EXPECT_EQ(hashMap1.size(), 0);
	gc->DestroyPendingKillObjs();
	EXPECT_EQ(gc->GetObjectCount(), 0);
}