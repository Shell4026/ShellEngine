#include "ComponentTest.h"

#include "Core/Logger.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

#include <iostream>

ComponentTest::ComponentTest()
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
	using namespace sh;
	using namespace sh::game;
	if (Input::GetKeyDown(Input::KeyCode::Enter))
	{
		gameObject->transform->SetParent(nullptr);
	}
	if (Input::GetKeyDown(Input::KeyCode::Space))
	{
		glm::vec3 pos = gameObject->transform->GetWorldPosition();
		SH_INFO_FORMAT("worldPosition: x: {}, y: {}, z: {}", pos.x, pos.y, pos.z);
	}
}

void ComponentTest::OnPropertyChanged(const sh::core::reflection::Property& prop)
{
	std::string* name = prop.Get<std::string>(this);
	auto obj = gameObject->world.GetGameObject(*name);
	if (obj)
	{
		gameObject->transform->SetParent(obj->transform);
	}
}
