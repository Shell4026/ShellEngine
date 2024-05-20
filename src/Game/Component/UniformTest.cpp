#include "Component/UniformTest.h"

#include "GameObject.h"

#include <glm/glm.hpp>

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
		glm::vec3 pos = gameObject.transform->position;
		glm::vec3 rot = gameObject.transform->rotation;
		gameObject.transform->SetRotation(rot + glm::vec3(0.0f, 45 * gameObject.world.deltaTime, 0.0f));
		//gameObject.transform->SetPosition(pos + glm::vec3(0.f, 0.2 * gameObject.world.deltaTime, 0.f));
	}
}