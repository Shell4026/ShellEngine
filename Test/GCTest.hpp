#pragma once

#include "../include/Core/SObject.h"
#include "../include/Core/GarbageCollection.h"
#include "../include/Core/Util.h"

#include <gtest/gtest.h>

#include <vector>
#include <memory>

class Object : public sh::core::SObject
{
	SCLASS(Object)
public:
	int num;
	PROPERTY(child)
	Object* child;
	PROPERTY(others)
	std::vector<Object*> others;

	Object(int num) : 
		num(num), child(nullptr)
	{

	}
	Object(Object&& other) noexcept :
		SObject(std::move(other)),
		num(other.num), child(other.child), others(std::move(other.others))
	{

	}
	~Object()
	{
		num = 0;
	}
};

TEST(GCTest, ValidTest) 
{
	using namespace sh::core;

	GarbageCollection& gc = *GarbageCollection::GetInstance();
	
	{
		Object obj1{ 1 };
		Object* obj1Child = new Object{ 2 };
		Object* obj2 = new Object{ 3 };
		Object* obj2Child = new Object{ 4 };
		Object* unusedObj = new Object{ 5 };
		Object* unusedObjChild = new Object{ 6 };
		gc.SetRootSet(&obj1);

		obj1.child = obj1Child;
		obj2->child = obj2Child;

		unusedObj->child = unusedObjChild;

		obj1.others.push_back(obj1Child);
		obj1.others.push_back(obj2);
		obj1.others.push_back(obj2Child);

		delete obj1Child; //강제 제거
		obj2->Destroy(); //GC를 이용한 제거

		EXPECT_TRUE(obj2->IsPendingKill());
		EXPECT_FALSE(IsValid(obj2));

		EXPECT_NE(obj1Child->num, 2);
		EXPECT_EQ(obj2->num, 3);
		EXPECT_EQ(obj2Child->num, 4);
		gc.Update();

		EXPECT_EQ(obj1.child, nullptr);
		EXPECT_NE(obj2->num, 3);
		EXPECT_EQ(obj2Child->num, 4); //부모는 죽었지만 벡터 내부에 살아있음
		EXPECT_NE(unusedObj->num, 5);
		EXPECT_NE(unusedObjChild->num, 6);

		EXPECT_EQ(obj1.others[0], nullptr);
		EXPECT_EQ(obj1.others[1], nullptr);
		EXPECT_NE(obj1.others[2], nullptr);
	}
	//obj2Child는 이제 접근 할 수 없다. 제거됨
	gc.Update();
	EXPECT_EQ(gc.GetObjectCount(), 0);
}