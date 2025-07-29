#pragma once
#include "../include/Core/SObject.h"
#include "../include/Core/Reflection.hpp"
#include "../include/Core/GarbageCollection.h"
#include "../include/Core/SContainer.hpp"

#include <gtest/gtest.h>
#include <vector>
#include <set>
#include <array>
#include <atomic>
/// GEMINI CLI로 생성한 테스트 코드

// 테스트용 기본 SObject
class TestObject : public sh::core::SObject
{
	SCLASS(TestObject)
public:
	int id;

	PROPERTY(child)
	TestObject* child = nullptr;

	PROPERTY(objectList)
	std::vector<TestObject*> objectList;

	PROPERTY(objectSet)
	std::set<TestObject*> objectSet;

	PROPERTY(objectArray)
	std::array<TestObject*, 3> objectArray{ nullptr, nullptr, nullptr };

	TestObject(int id = 0) : id(id) {}
	~TestObject() override {
		// 소멸 시 id를 0으로 만들어 소멸되었음을 외부에서 확인할 수 있도록 함
		id = 0;
	}
};

// 소멸 순서 테스트를 위한 클래스
class TestWorld; // Forward declaration

// 전역 플래그로 World의 생존 여부 추적
static std::atomic<bool> isWorldAlive = false;
// World보다 먼저 파괴된 GameObject 수
static std::atomic<int> destroyedGameObjectsCount = 0;

class TestGameObject : public sh::core::SObject
{
	SCLASS(TestGameObject)
public:
	// 자식 객체는 부모 객체에 대한 포인터를 가질 수 있음
	TestWorld* world = nullptr;

	TestGameObject(TestWorld* owner) : world(owner) {}

	~TestGameObject() override
	{
		// 이 객체가 파괴될 때, World는 반드시 살아있어야 함
		if (isWorldAlive)
		{
			destroyedGameObjectsCount++;
		}
	}
};
class TestWorld : public sh::core::SObject
{
	SCLASS(TestWorld)
public:
	PROPERTY(gameObjects)
	std::vector<TestGameObject*> gameObjects;

	TestWorld()
	{
		isWorldAlive = true;
	}

	~TestWorld() override
	{
		isWorldAlive = false;
	}

	void AddGameObject(TestGameObject* obj)
	{
		gameObjects.push_back(obj);
	}

	void OnDestroy() override
	{
		for (auto obj : gameObjects)
			obj->Destroy();
		Super::OnDestroy();
	}
};

// 테스트 환경 초기화 및 정리를 위한 Test Fixture
class GCTest : public ::testing::Test {
protected:
	sh::core::GarbageCollection* gc;

	void SetUp() override {
		gc = sh::core::GarbageCollection::GetInstance();
		// 각 테스트 시작 전, 이전 테스트에서 남은 객체가 없도록 GC를 실행
		gc->Collect();
		gc->Collect(); // pendingKill 객체까지 완전히 정리
		ASSERT_EQ(gc->GetObjectCount(), 0);
	}

	void TearDown() override {
		// 각 테스트 종료 후, 생성된 모든 객체가 정리되도록 보장
		gc->Collect();
		gc->Collect();
	}
};

// 1. 기본 GC 테스트: 아무도 참조하지 않는 객체는 수집되어야 한다.
TEST_F(GCTest, ShouldCollectUnreferencedObject)
{
	TestObject* obj = sh::core::SObject::Create<TestObject>(1);
	ASSERT_EQ(gc->GetObjectCount(), 1);
	ASSERT_TRUE(IsValid(obj));

	// GC 실행
	gc->Collect();
	gc->Collect();

	// 객체가 수집되었는지 확인
	ASSERT_EQ(gc->GetObjectCount(), 0);
}

// 2. 루트 객체 테스트: 루트로 지정된 객체는 수집되지 않아야 한다.
TEST_F(GCTest, ShouldNotCollectRootObject)
{
	TestObject* root = sh::core::SObject::Create<TestObject>(100);
	gc->SetRootSet(root);
	ASSERT_EQ(gc->GetObjectCount(), 1);

	gc->Collect();
	gc->Collect();

	// 루트 객체는 살아남아야 함
	ASSERT_EQ(gc->GetObjectCount(), 1);
	ASSERT_TRUE(IsValid(root));
	ASSERT_EQ(root->id, 100);

	// 루트에서 제거 후 정리
	gc->RemoveRootSet(root);
}

// 3. 참조 객체 테스트: 루트가 참조하는 객체는 수집되지 않아야 한다.
TEST_F(GCTest, ShouldNotCollectReferencedObject)
{
	TestObject* root = sh::core::SObject::Create<TestObject>(100);
	TestObject* child = sh::core::SObject::Create<TestObject>(101);
	root->child = child;
	gc->SetRootSet(root);

	ASSERT_EQ(gc->GetObjectCount(), 2);

	gc->Collect();
	gc->Collect();

	// 루트와 자식 객체 모두 살아남아야 함
	ASSERT_EQ(gc->GetObjectCount(), 2);
	ASSERT_TRUE(IsValid(root));
	ASSERT_TRUE(IsValid(child));
	ASSERT_EQ(child->id, 101);

	gc->RemoveRootSet(root);
}

// 4. 참조 해제 테스트: 루트의 참조가 끊기면 객체는 수집되어야 한다.
TEST_F(GCTest, ShouldCollectWhenReferenceIsRemoved)
{
	TestObject* root = sh::core::SObject::Create<TestObject>(100);
	TestObject* child = sh::core::SObject::Create<TestObject>(101);
	root->child = child;
	gc->SetRootSet(root);

	gc->Collect();
	gc->Collect();
	ASSERT_EQ(gc->GetObjectCount(), 2);

	// 참조를 끊음
	root->child = nullptr;
	gc->Collect();
	gc->Collect();

	// 자식 객체만 수집되어야 함
	ASSERT_EQ(gc->GetObjectCount(), 1);
	ASSERT_TRUE(IsValid(root));
	ASSERT_FALSE(IsValid(child)); // IsValid는 pendingKill 상태를 확인

	gc->RemoveRootSet(root);
}

// 5. 순환 참조 테스트: 순환 참조가 있어도 루트에서 도달 불가능하면 수집되어야 한다.
TEST_F(GCTest, ShouldCollectCircularReferences)
{
	TestObject* obj1 = sh::core::SObject::Create<TestObject>(1);
	TestObject* obj2 = sh::core::SObject::Create<TestObject>(2);

	// 순환 참조 생성: obj1 -> obj2 -> obj1
	obj1->child = obj2;
	obj2->child = obj1;

	ASSERT_EQ(gc->GetObjectCount(), 2);

	gc->Collect();
	gc->Collect();

	// 루트에서 접근 불가능하므로 둘 다 수집되어야 함
	ASSERT_EQ(gc->GetObjectCount(), 0);
}

// 6. 컨테이너 참조 테스트: 컨테이너(vector) 내의 객체들도 올바르게 추적되어야 한다.
TEST_F(GCTest, ShouldHandleReferencesInVector)
{
	TestObject* root = sh::core::SObject::Create<TestObject>(100);
	gc->SetRootSet(root);

	for (int i = 1; i <= 5; ++i) {
		root->objectList.push_back(sh::core::SObject::Create<TestObject>(i));
	}
	ASSERT_EQ(gc->GetObjectCount(), 6);

	gc->Collect();
	gc->Collect();
	ASSERT_EQ(gc->GetObjectCount(), 6); // 모두 살아남아야 함

	// 벡터의 일부 참조를 제거
	TestObject* objToCollect = root->objectList[2]; // id = 3
	root->objectList.erase(root->objectList.begin() + 2);
	ASSERT_EQ(root->objectList.size(), 4);

	gc->Collect();
	gc->Collect();

	// objToCollect는 수집되어야 함
	ASSERT_EQ(gc->GetObjectCount(), 5);
	ASSERT_FALSE(IsValid(objToCollect));

	gc->RemoveRootSet(root);
}

// 7. 소멸 순서 테스트: 자식 객체가 부모 객체보다 먼저 소멸되어야 한다.
TEST_F(GCTest, ShouldDestroyChildrenBeforeParent)
{
	destroyedGameObjectsCount = 0;
	isWorldAlive = false;

	// 1. 월드와 게임 오브젝트 생성
	TestWorld* world = sh::core::SObject::Create<TestWorld>();
	constexpr int numGameObjects = 5;
	for (int i = 0; i < numGameObjects; ++i)
	{
		world->AddGameObject(sh::core::SObject::Create<TestGameObject>(world));
	}
	ASSERT_TRUE(isWorldAlive);
	ASSERT_EQ(gc->GetObjectCount(), 1 + numGameObjects);

	// 2. 월드를 루트로 설정하고 GC 실행 -> 아무것도 수집되지 않아야 함
	gc->SetRootSet(world);
	gc->Collect();
	gc->Collect();
	ASSERT_EQ(gc->GetObjectCount(), 1 + numGameObjects);

	// 3. 월드를 루트에서 제거 -> 이제 모두 GC 대상이 됨
	gc->RemoveRootSet(world);
	
	// 4. GC 실행하여 객체 소멸 유도
	gc->Collect();
	gc->Collect();

	// 5. 검증
	// World가 파괴되기 전에 모든 GameObject가 파괴되었어야 함
	ASSERT_EQ(destroyedGameObjectsCount, numGameObjects);
	// 모든 객체가 소멸되었는지 확인
	ASSERT_EQ(gc->GetObjectCount(), 0);
	ASSERT_FALSE(isWorldAlive);
}