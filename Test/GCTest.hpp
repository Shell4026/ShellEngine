#pragma once

#include "../include/Core/SObject.h"
#include "../include/Core/GC.h"
#include "../include/Core/Util.h"

#include <gtest/gtest.h>

class Object : public sh::core::SObject
{
	SCLASS(Object)
public:
	int num;
	PROPERTY(other)
	Object* other;
	Object(int num) : 
		num(num), other(nullptr)
	{

	}
	~Object()
	{

	}
};
TEST(GCTest, HeapTest)
{
	Object obj0{ 1 };
	Object* obj1 = new Object{ 2 };

	EXPECT_FALSE(obj0.isHeap);
	EXPECT_TRUE(obj1->isHeap);

	delete obj1;
}
TEST(GCTest, ValidTest) 
{
	using namespace sh::core;

	GC gc;

	Object obj0{ 1 };
	obj0.SetGC(gc);
	Object* obj1 = new Object{ 2 };
	obj1->SetGC(gc);

	obj0.other = obj1;

	EXPECT_TRUE(IsValid(obj1));
	EXPECT_FALSE(obj1->IsPendingKill());
	delete obj1;
	EXPECT_NE(obj0.other, nullptr);
	EXPECT_FALSE(IsValid(obj1));
	EXPECT_TRUE(obj1->IsPendingKill());
	EXPECT_EQ(obj1->num, 2);

	gc.Update();
	EXPECT_EQ(obj0.other, nullptr);
	EXPECT_FALSE(IsValid(obj1));
}