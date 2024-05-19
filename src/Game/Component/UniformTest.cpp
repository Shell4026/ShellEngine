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
		t += 45.0f * gameObject.world.deltaTime;
		if (t > 360.f)
			t -= 360.f;
		glm::vec4& offset = *mat->GetVector("offset");

		mat->SetVector("offset", glm::vec4{ 0.5 * glm::cos(glm::radians(t)), 0.5 * glm::sin(glm::radians(t)), 0.0f, 0.0f});
	}
}