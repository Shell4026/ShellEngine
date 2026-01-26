#include "Component/TestComponent.h"

#include "Game/World.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/Component/Render/MeshRenderer.h"

#include "Render/Mesh.h"

#include <random>

namespace sh::game
{
	Test::Test(GameObject& owner) :
		Component(owner)
	{
	}

	SH_GAME_API void Test::Awake()
	{
		SH_INFO("Awake!");
	}

	SH_GAME_API void Test::Start()
	{
		SH_INFO("Start!");
	}

	SH_GAME_API void Test::OnEnable()
	{
		SH_INFO_FORMAT("Enable! {}", gameObject.GetName().ToString());
	}

	SH_GAME_API void Test::OnDisable()
	{
		SH_INFO_FORMAT("Disable... {}", gameObject.GetName().ToString());
	}

	SH_GAME_API void Test::OnTriggerEnter(Collider& other)
	{
		SH_INFO("enter!");
	}
	SH_GAME_API void Test::OnTriggerStay(Collider& other)
	{
		//SH_INFO("stay!");
	}
	SH_GAME_API void Test::OnTriggerExit(Collider& other)
	{
		SH_INFO("exit!");
	}
	SH_GAME_API void Test::Update()
	{
		if (Input::GetKeyPressed(Input::KeyCode::Space))
		{
			for (int i = 0; i < num; ++i)
			{
				auto obj = world.AddGameObject("test");
				auto meshr = obj->AddComponent<MeshRenderer>();
				auto modelPtr = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f15" }));
				meshr->SetMesh(modelPtr->GetMeshes()[0]);
			}
		}
		if (Input::GetKeyPressed(Input::KeyCode::K))
		{
			SH_INFO("Change constant");
			bUseTexture = !bUseTexture;
			mat->SetConstant("USE_TEXTURE", bUseTexture);
		}
	}
}//namespace