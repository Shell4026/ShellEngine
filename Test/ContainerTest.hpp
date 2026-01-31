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

	{
		SObject* objInContainer = SObject::Create<SObject>();
		SObject* objOutContainer = SObject::Create<SObject>();
		SVector<const SObject*> vector;
		vector.push_back(objInContainer);
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_FALSE(IsValid(objOutContainer));
		EXPECT_TRUE(vector[0] != nullptr);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 vector의 원소가 nullptr로 바뀜
		EXPECT_TRUE(vector[0] == nullptr);
		EXPECT_EQ(gc->GetObjectCount(), 2);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SArray<const SObject*, 1> arr;
		arr[0] = objInContainer;
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(arr[0] != nullptr);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 arr의 원소가 nullptr로 바뀜
		EXPECT_TRUE(arr[0] == nullptr);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SList <const SObject*> list;
		list.push_back(objInContainer);
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(*list.begin() != nullptr);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 list의 원소가 제거됨
		EXPECT_EQ(list.size(), 0);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SSet<const SObject*> set;
		set.insert(objInContainer);
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(set.size() == 1);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 set의 원소가 제거됨
		EXPECT_TRUE(set.size() == 0);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SHashSet<const SObject*> set;
		set.insert(objInContainer);
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(set.size() == 1);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 set의 원소가 제거됨
		EXPECT_TRUE(set.size() == 0);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SMap<const SObject*, int> map0;
		SMap<int, const SObject*> map1;
		map0.insert({ objInContainer, 1 });
		map1.insert({ 1, objInContainer });
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(map0.size() == 1);
		EXPECT_TRUE(map1.size() == 1);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 map의 원소가 제거됨
		EXPECT_EQ(map0.size(), 0);
		EXPECT_EQ(map1.size(), 0);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	{
		SObject* objInContainer = SObject::Create<SObject>();
		SHashMap<const SObject*, int> map0;
		SHashMap<int, const SObject*> map1;
		map0.insert({ objInContainer, 1 });
		map1.insert({ 1, objInContainer });
		gc->Collect();
		EXPECT_TRUE(IsValid(objInContainer));
		EXPECT_TRUE(map0.size() == 1);
		EXPECT_TRUE(map1.size() == 1);
		objInContainer->Destroy();
		EXPECT_FALSE(IsValid(objInContainer));
		gc->Collect(); // 여기서 map의 원소가 제거됨
		EXPECT_EQ(map0.size(), 0);
		EXPECT_EQ(map1.size(), 0);
		EXPECT_EQ(gc->GetObjectCount(), 1);
		gc->DestroyPendingKillObjs();
		EXPECT_EQ(gc->GetObjectCount(), 0);
	}
	EXPECT_EQ(gc->GetTrackedContainerCount(), 0);

	//SHashMap<SObject*, int> hashMap0;
	//SHashMap<int, SObject*> hashMap1;
	//hashMap0[objInContainer] = 0;
	//hashMap1[0] = objInContainer;
	//EXPECT_EQ(gc->GetObjectCount(), 2);
	//gc->Collect();
	//EXPECT_TRUE(IsValid(objInContainer));
	//EXPECT_FALSE(IsValid(objOutContainer));
	//gc->DestroyPendingKillObjs();
	//EXPECT_EQ(gc->GetObjectCount(), 1);
	//EXPECT_EQ(hashMap0.size(), 1);
	//EXPECT_EQ(hashMap1.size(), 1);
	//objInContainer->Destroy();
	//EXPECT_FALSE(IsValid(objInContainer));
	//gc->Collect(); // 여기서 hashMap0,1의 원소가 제거됨
	//EXPECT_EQ(hashMap0.size(), 0);
	//EXPECT_EQ(hashMap1.size(), 0);
	//gc->DestroyPendingKillObjs();
	//EXPECT_EQ(gc->GetObjectCount(), 0);
}