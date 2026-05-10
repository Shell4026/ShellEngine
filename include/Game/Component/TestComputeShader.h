#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

namespace sh::render
{
	class ComputeShader;
}

namespace sh::game
{
	class TestComputeShader : public Component
	{
		COMPONENT(TestComputeShader)
	public:
		SH_GAME_API TestComputeShader(GameObject& owner);

		SH_GAME_API void Update() override;
	private:
		PROPERTY(shader)
		render::ComputeShader* shader = nullptr;
	};
}//namespace
