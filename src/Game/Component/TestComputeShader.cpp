#include "Component/TestComputeShader.h"

#include "Game/World.h"
#include "Game/Input.h"

#include "Render/Renderer.h"
#include "Render/ScriptableRenderer.h"
#include "Render/ComputeShader.h"

#include "Core/Logger.h"

namespace sh::game
{
	TestComputeShader::TestComputeShader(GameObject& owner) :
		Component(owner)
	{
	}

	SH_GAME_API void TestComputeShader::Update()
	{
		if (!Input::GetKeyPressed(Input::KeyCode::C))
			return;

		if (shader == nullptr)
		{
			SH_ERROR("TestComputeShader: shader is null");
			return;
		}

		render::ScriptableRenderer* const scriptableRenderer = world.renderer.GetScriptableRenderer();
		if (scriptableRenderer == nullptr)
		{
			SH_ERROR("TestComputeShader: ScriptableRenderer is null");
			return;
		}
		std::vector<float> a(10'000, 0);
		std::vector<float> b(10'000, 0);
		std::vector<float> c(10'000, 0);
		for (int i = 0; i < 10'000; ++i)
		{
			a[i] = i;
			b[i] = 10'000 - i;
		}
		shader->SetFloats("bufferA", a.data(), a.size());
		shader->SetFloats("bufferB", b.data(), b.size());
		shader->SetFloats("bufferC", c.data(), c.size());
		const uint32_t workGroup = std::ceil(10'000.f / 128.f);
		SH_INFO_FORMAT("Dispatch ComputeShader [{} x {} x {}]", workGroup, 1, 1);
		scriptableRenderer->Dispatch(*shader, workGroup, 1, 1);
		std::vector<uint8_t> buffer = shader->GetBuffer("bufferC")->GetData();
		std::memcpy(c.data(), buffer.data(), buffer.size());
	}
}//namespace
