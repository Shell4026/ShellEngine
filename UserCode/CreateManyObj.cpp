#include "CreateManyObj.h"

#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/Input.h"
#include "Game/Component/MeshRenderer.h"

#include <random>

using namespace sh::game;
CreateManyObj::CreateManyObj(sh::game::GameObject& owner) :
	Component(owner)
{

}

SH_USER_API void CreateManyObj::Update()
{
	static bool keyDown = false;

	if (!keyDown)
	{
		if (Input::GetKeyDown(Input::KeyCode::Enter))
		{
			keyDown = true;
			for (int i = 0; i < count; ++i)
			{
				auto obj = world.AddGameObject("Test");
				std::random_device device{};
				std::mt19937 seed{ device()};
				
				std::uniform_real<float> rand{ -10.f, 10.f };

				obj->transform->SetPosition(rand(seed), rand(seed), rand(seed));
				auto renderer = obj->AddComponent<MeshRenderer>();
				renderer->SetMesh(*world.meshes.GetResource("Cube"));
			}
		}
	}
	else
	{
		if (!Input::GetKeyDown(Input::KeyCode::Enter))
		{
			keyDown = false;
		}
	}
}
