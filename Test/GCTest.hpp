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
	std::vector<std::vector<Object*>> others;
	PROPERTY(childs)
	sh::core::SSet<Object*> childs;
	PROPERTY(arr)
	std::array<Object*, 2> arr{ nullptr, nullptr };
	PROPERTY(nested)
	std::array<std::vector<Object*>, 2> nested;

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

	Object* root = SObject::Create<Object>(1);
	gc.SetRootSet(root);
	Object* child = SObject::Create<Object>(2);
	root->child = child;

	EXPECT_TRUE(IsValid(root));
	EXPECT_TRUE(IsValid(child));
	gc.Update();
	EXPECT_TRUE(IsValid(root));
	EXPECT_TRUE(IsValid(child));
	EXPECT_FALSE(root->IsPendingKill());
	EXPECT_FALSE(child->IsPendingKill());

	child->Destroy();
	EXPECT_TRUE(child->IsPendingKill());
	EXPECT_FALSE(IsValid(child));
	EXPECT_EQ(child->num, 2); // 아직은 child에 접근 할 수 있음

	gc.Update();
	EXPECT_NE(child->num, 2); // 어떤 값이 될지는 알 수 없지만 지워짐
	EXPECT_EQ(root->child, nullptr); // 소멸된 값은 자동으로 nullptr가 된다.

	Object* dummy = nullptr;
	Object* newChild = nullptr;
	{
		dummy = SObject::Create<Object>(3);
		newChild = SObject::Create<Object>(4);
		root->child = newChild;
	}
	EXPECT_EQ(dummy->num, 3);
	gc.Update();
	EXPECT_NE(dummy->num, 3); // 접근 할 수 없음 = 지워짐
	EXPECT_EQ(newChild->num, 4);

	root->Destroy();
	gc.Update();

	EXPECT_NE(root->num, 1);
	EXPECT_NE(newChild->num, 4);
}

TEST(GCTest, CircularReferencingTest)
{
	using namespace sh::core;
	GarbageCollection& gc = *GarbageCollection::GetInstance();

	Object* root = SObject::Create<Object>(999);
	Object* obj1 = SObject::Create<Object>(1);
	Object* obj2 = SObject::Create<Object>(2);

	gc.SetRootSet(root);

	root->child = obj1;
	obj1->child = obj2;
	obj2->child = obj1;

	root->Destroy();
	gc.Update();

	EXPECT_NE(obj1->num, 1);
	EXPECT_NE(obj2->num, 2);
}

TEST(GCTest, ContainerTest) 
{
	using namespace sh::core;

	GarbageCollection& gc = *GarbageCollection::GetInstance();

	Object* root = SObject::Create<Object>(1);
	gc.SetRootSet(root);
	
	for (int i = 0; i < 3; ++i)
	{
		std::vector<Object*> objs(3);
		for (int j = 0; j < 3; ++j)
		{
			objs[j] = SObject::Create<Object>(i * 3 + j);
		}
		root->others.push_back(std::move(objs));
	}

	EXPECT_EQ(gc.GetObjectCount(), 10);
	gc.Update();
	EXPECT_EQ(gc.GetObjectCount(), 10);

	Object* temp = root->others[1][1];

	root->Destroy();
	gc.Update();
	EXPECT_NE(temp->num, 4);

	// SSet테스트
	Object* setRoot = SObject::Create<Object>(123);
	gc.SetRootSet(setRoot);
	{
		Object* root = SObject::Create<Object>(999);
		gc.SetRootSet(root);
		for (int i = 0; i < 3; ++i)
			root->childs.insert(SObject::Create<Object>(i));

		gc.Update();

		for (auto child : root->childs)
		{
			child->Destroy();
		}

		EXPECT_EQ(root->childs.size(), 3);
		gc.Update();
		EXPECT_EQ(root->childs.size(), 0);

		root->Destroy();
		root->childs.insert(SObject::Create<Object>(4));
		setRoot->child = *root->childs.begin();
	}
	EXPECT_EQ(setRoot->child->num, 4);
	setRoot->Destroy();
	gc.Update();
	EXPECT_EQ(gc.GetObjectCount(), 0);
	// Array 테스트
	{
		Object* arrRoot = SObject::Create<Object>(123);
		gc.SetRootSet(arrRoot);

		arrRoot->arr[0] = SObject::Create<Object>(1);
		arrRoot->arr[1] = SObject::Create<Object>(2);

		gc.Update();

		EXPECT_EQ(arrRoot->arr[0]->num, 1);
		EXPECT_EQ(arrRoot->arr[1]->num, 2);

		arrRoot->arr[1]->Destroy();
		gc.Update();

		EXPECT_EQ(arrRoot->arr[1], nullptr);
		arrRoot->Destroy();
		gc.Update();
	}
	EXPECT_EQ(gc.GetObjectCount(), 0);
	// 중첩 컨테이너 테스트
	{
		Object* root = SObject::Create<Object>(24);
		gc.SetRootSet(root);

		root->nested[0].push_back(SObject::Create<Object>(1));
		root->nested[0].push_back(SObject::Create<Object>(2));
		root->nested[1].push_back(SObject::Create<Object>(3));
		root->nested[1].push_back(SObject::Create<Object>(4));

		gc.Update();

		EXPECT_EQ(root->nested[1][0]->num, 3);

		root->nested[1][1]->Destroy();
		gc.Update();

		EXPECT_EQ(root->nested[1].size(), 1); // 벡터라서 원소가 지워짐

		root->Destroy();
		gc.Update();
	}
	EXPECT_EQ(gc.GetObjectCount(), 0);
}