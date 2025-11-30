#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

#include "Core/SContainer.hpp"
#include "Render/Material.h"

namespace sh::game
{
	class Test : public Component
	{
		COMPONENT(Test)
	public:
		SH_GAME_API Test(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;

		SH_GAME_API void OnEnable() override;
		SH_GAME_API void OnDisable() override;

		SH_GAME_API void OnTriggerEnter(Collider& other) override;
		SH_GAME_API void OnTriggerStay(Collider& other) override;
		SH_GAME_API void OnTriggerExit(Collider& other) override;

		SH_GAME_API void Update() override;
	private:
		PROPERTY(num)
		int num = 10;
		PROPERTY(mat)
		render::Material* mat = nullptr;
		
		bool bUseTexture = true;
	};
}
