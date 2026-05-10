#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"

#include "Render/RenderData.h"

#include <array>
namespace sh::render
{
	class Material;
	class RenderTexture;
	class Texture;
}

namespace sh::game
{
	class Camera;

	/// @brief 카메라에 부착해 SSAO 를 활성화한다.
	class SSAOComponent : public Component
	{
		COMPONENT(SSAOComponent)
	public:
		SH_GAME_API SSAOComponent(GameObject& owner);
		SH_GAME_API ~SSAOComponent();

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API void SetMaterial(render::Material* mat);

		auto GetDepthRT() const -> render::RenderTexture* { return depthRT; }
		auto GetAORT() const -> render::RenderTexture* { return aoRT; }
		auto GetRadius() const -> float { return radius; }
		auto GetBias() const -> float { return bias; }
		auto GetPower() const -> float { return power; }
	private:
		void EnsureRenderTextures();
		void UpdateMaterialUniforms();
		static void CreateKernel();
	private:
		PROPERTY(mat, core::PropertyOption::sobjPtr)
		render::Material* mat = nullptr;

		PROPERTY(radius)
		float radius = 0.5f;
		PROPERTY(bias)
		float bias = 0.025f;
		PROPERTY(power)
		float power = 1.5f;

		PROPERTY(depthRT, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		render::RenderTexture* depthRT = nullptr;
		PROPERTY(normalRT, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		render::RenderTexture* normalRT = nullptr;
		PROPERTY(aoRT, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		render::RenderTexture* aoRT = nullptr;
		PROPERTY(camera, core::PropertyOption::sobjPtr, core::PropertyOption::invisible);
		Camera* camera = nullptr;
		PROPERTY(noiseTex, core::PropertyOption::sobjPtr, core::PropertyOption::invisible);
		render::Texture* noiseTex = nullptr;

		render::RenderData depthRenderData;
		render::RenderData ssaoRenderData;
		render::RenderData combineRenderData;

		uint32_t cachedWidth = 0;
		uint32_t cachedHeight = 0;

		static std::array<glm::vec3, 64> kernel;
		static bool bKernelInit;
	};
}//namespace
