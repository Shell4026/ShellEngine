#include "UniformTest.h"

#include "Game/GameObject.h"
#include "Game/Input.h"

namespace sh::game
{
	UniformTest::UniformTest(GameObject& owner) :
		Component(owner),
		mat(nullptr)
	{
	}

	void UniformTest::SetMaterial(sh::render::Material& mat)
	{
		this->mat = &mat;
	}

	void UniformTest::Update()
	{
		if (Input::GetKeyDown(Input::KeyCode::A))
		{
			auto pos = gameObject.transform->position;
			pos.x -= 1 * gameObject.world.deltaTime;
			gameObject.transform->SetPosition(pos);
		}
		if (Input::GetKeyDown(Input::KeyCode::D))
		{
			auto pos = gameObject.transform->position;
			pos.x += 1 * gameObject.world.deltaTime;
			gameObject.transform->SetPosition(pos);
		}
		if (Input::GetKeyDown(Input::KeyCode::W))
		{
			auto pos = gameObject.transform->position;
			pos.z -= 1 * gameObject.world.deltaTime;
			gameObject.transform->SetPosition(pos);
		}
		if (Input::GetKeyDown(Input::KeyCode::S))
		{
			auto pos = gameObject.transform->position;
			pos.z += 1 * gameObject.world.deltaTime;
			gameObject.transform->SetPosition(pos);
		}
		Vec3 pos = gameObject.transform->position;
		Vec3 rot = gameObject.transform->rotation;
		gameObject.transform->SetRotation(rot + Vec3{ 0.0f, 45 * gameObject.world.deltaTime, 0.0f });
		//gameObject.transform->SetPosition(pos + glm::vec3(0.f, 0.2 * gameObject.world.deltaTime, 0.f));
	}
}