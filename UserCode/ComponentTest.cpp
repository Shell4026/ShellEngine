#include "ComponentTest.h"

#include "Game/GameObject.h"

#include <iostream>

ComponentTest::ComponentTest() :
	height(1.0f)
{
}

ComponentTest::~ComponentTest()
{
}

void ComponentTest::OnEnable()
{
	std::cout << "Test: Enable!\n";
}

void ComponentTest::Update()
{
	glm::vec3 pos = gameObject->transform->position;
	if (pos.y < height)
	{
		gameObject->transform->SetPosition(pos.x, pos.y + 0.1f * gameObject->world.deltaTime, pos.z);
	}
}
