#include "Save.h"
#include "Game/Input.h"
#include "Game/GameObject.h"

#include <fstream>

using namespace sh::game;

Save::Save(sh::game::GameObject& owner) :
	Component(owner)
{
}

void Save::Update()
{
	if (Input::GetKeyDown(Input::KeyCode::LCtrl) && Input::GetKeyDown(Input::KeyCode::S))
	{
		std::ofstream file{ "test.json" };
		file << gameObject.Serialize();
	}
	if (Input::GetKeyDown(Input::KeyCode::LCtrl) && Input::GetKeyDown(Input::KeyCode::O))
	{
		std::ifstream file{ "test.json" };
		sh::core::Json json{};
		file >> json;
		gameObject.Deserialize(json);
	}
}
