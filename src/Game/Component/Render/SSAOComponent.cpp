#include "Component/Render/SSAOComponent.h"
#include "Component/Render/Camera.h"
#include "World.h"
#include "GameObject.h"
#include "GameRenderer.h"

#include "Core/SObject.h"
#include "Core/GarbageCollection.h"
#include "Core/Util.h"

#include "Render/Renderer.h"
#include "Render/IRenderContext.h"
#include "Render/Material.h"
#include "Render/RenderTexture.h"
#include "Render/RenderPass/SSAOPass.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/packing.hpp>

#include <cstdint>
#include <random>
namespace sh::game
{
	std::array<glm::vec3, 64> SSAOComponent::kernel{};
	bool SSAOComponent::bKernelInit = false;

	SSAOComponent::SSAOComponent(GameObject& owner) :
		Component(owner)
	{
		canPlayInEditor = true;

		depthRenderData.tag = core::Name{ "Depth" };
		ssaoRenderData.tag = core::Name{ "SSAO" };
	}
	SSAOComponent::~SSAOComponent() = default;

	SH_GAME_API void SSAOComponent::Awake()
	{
		camera = gameObject.GetComponent<Camera>();
		if (camera == nullptr)
			SH_ERROR("SSAOComponent requires a Camera component on the same GameObject");
		CreateKernel();
		SetMaterial(static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(core::UUID{ "bbc4ef7ec45dce223297a224f8093f24" })));
	}
	SH_GAME_API void SSAOComponent::OnDestroy()
	{
		if (depthRT != nullptr)
			depthRT->Destroy();
		if (aoRT != nullptr)
			aoRT->Destroy();

		Super::OnDestroy();
	}
	SH_GAME_API void SSAOComponent::BeginUpdate()
	{
		if (camera == nullptr)
			return;

		depthRenderData.renderViewers = ssaoRenderData.renderViewers = camera->GetRenderData().renderViewers;
		EnsureRenderTextures();
		UpdateMaterialUniforms();

		world.renderer.PushRenderData(depthRenderData);
		world.renderer.PushRenderData(ssaoRenderData);
	}
	SH_GAME_API void SSAOComponent::SetMaterial(render::Material* _mat)
	{
		mat = _mat;
		if (mat == nullptr)
			return;

		GameRenderer* const renderer = dynamic_cast<GameRenderer*>(world.renderer.GetScriptableRenderer());
		if (renderer == nullptr)
			return;

		render::SSAOPass& ssaoPass = renderer->GetSSAOPass();
		ssaoPass.SetMaterial(*mat);
	}

	void SSAOComponent::EnsureRenderTextures()
	{
		uint32_t w = static_cast<uint32_t>(camera->GetWidth());
		uint32_t h = static_cast<uint32_t>(camera->GetHeight());
		if (w == 0 || h == 0)
			return;

		render::IRenderContext* ctx = world.renderer.GetContext();
		if (ctx == nullptr)
			return;

		core::GarbageCollection& gc = *core::GarbageCollection::GetInstance();

		if (depthRT == nullptr)
		{
			depthRT = core::SObject::Create<render::RenderTexture>(render::TextureFormat::None, render::TextureFormat::D32, false);
			depthRT->SetSize(w, h);
			depthRT->Build(*ctx);
		}
		if (normalRT == nullptr)
		{
			normalRT = core::SObject::Create<render::RenderTexture>(render::TextureFormat::RGBA32, render::TextureFormat::None, false);
			normalRT->SetSize(w, h);
			normalRT->Build(*ctx);
		}
		depthRenderData.SetRenderTargets({ depthRT, normalRT });

		if (aoRT == nullptr)
		{
			aoRT = core::SObject::Create<render::RenderTexture>(render::TextureFormat::R8, render::TextureFormat::None, false);
			aoRT->SetSize(w, h);
			aoRT->Build(*ctx);

			ssaoRenderData.SetRenderTarget(aoRT);
		}

		if (noiseTex == nullptr)
		{
			noiseTex = core::SObject::Create<render::Texture>(render::TextureFormat::RG32F, 4, 4, false);
			std::array<uint16_t, 32> noise{}; // 4x4x2 channels, R16G16_SFLOAT
			for (int i = 0; i < 32; ++i)
				noise[i] = glm::packHalf1x16(core::Util::RandomRange(-1.f, 1.f));
			noiseTex->SetPixelData(reinterpret_cast<const uint8_t*>(noise.data()), sizeof(uint16_t) * noise.size());
			noiseTex->SetAnisoLevel(0);
			noiseTex->SetFiltering(render::Texture::Filtering::Box);
			noiseTex->Build(*ctx);
		}

		if (w != cachedWidth || h != cachedHeight)
		{
			depthRT->SetSize(w, h);
			normalRT->SetSize(w, h);
			aoRT->SetSize(w, h);
			cachedWidth = w;
			cachedHeight = h;
		}
	}
	void SSAOComponent::UpdateMaterialUniforms()
	{
		if (!core::IsValid(mat))
			return;

		const glm::mat4& proj = camera->GetProjMatrix();
		const glm::mat4 invProj = glm::inverse(proj);

		mat->SetProperty("kernel", kernel);
		mat->SetProperty("depthTex", depthRT);
		mat->SetProperty("normalTex", normalRT);
		mat->SetProperty("noiseTex", noiseTex);
		mat->UpdateUniformBuffers();
		//material->SetProperty("invProj", invProj);
		//material->SetProperty("proj", proj);
		//material->SetProperty("radius", radius);
		//material->SetProperty("bias", bias);
		//material->SetProperty("power", power);
		//material->SetProperty("depthTex", static_cast<const render::RenderTexture*>(depthRT));
	}
	void SSAOComponent::CreateKernel()
	{
		if (bKernelInit)
			return;
		bKernelInit = true;

		for (int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				core::Util::RandomRange(-1.f, 1.f),
				core::Util::RandomRange(-1.f, 1.f),
				core::Util::RandomRange(0.f, 1.f)
			);
			sample = glm::normalize(sample);
			sample *= core::Util::RandomRange(0.f, 1.f);
			float scale = i / 64.f;
			scale = glm::mix(0.1f, 1.0f, scale * scale);
			sample *= scale;
			kernel[i] = sample;
		}
	}
}//namespace
