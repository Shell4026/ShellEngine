#include "GameRenderer.h"
#include "UIPass.h"
#include "World.h"

#include "Render/RenderPass/TransparentPass.h"
namespace sh::game
{
	GameRenderer::GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, World& world) :
		ScriptableRenderer(ctx),
		renderCtx(ctx),
		world(world),
		guiCtx(guictx)
	{
	}
	SH_GAME_API void GameRenderer::Init()
	{
		AddShadowPass();
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::UI);
		AddRenderPass<game::UIPass>().SetImGUIContext(guiCtx);
	}
	SH_GAME_API void GameRenderer::Setup(const render::RenderTarget& data)
	{
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			auto allowedIt = allowedCamera.find(pass->passName);
			if (allowedIt != allowedCamera.end())
			{
				const std::set<const render::Camera*>& allowedCams = allowedIt->second;
				bool bAllowed = allowedCams.find(data.camera) != allowedCams.end();
				if (!bAllowed)
					continue;
			}

			auto it = ignoreCamera.find(pass->passName);
			if (it == ignoreCamera.end())
			{
				EnqueRenderPass(*pass);
				continue;
			}
			const std::set<const render::Camera*>& ignoreCams = it->second;
			bool bIgnore = ignoreCams.find(data.camera) != ignoreCams.end();
			if (bIgnore)
				continue;
			EnqueRenderPass(*pass);
		}
	}
	SH_GAME_API void GameRenderer::SetUICamera(const render::Camera& camera)
	{
		uiCamera = &camera;
		allowedCamera[core::Name{ "UI" }].insert(&camera);
		allowedCamera[core::Name{ "ImGUI" }].insert(&camera);
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass->passName == "UI" || pass->passName == "ImGUI")
				continue;
			ignoreCamera[pass->passName].insert(&camera);
		}
	}

	SH_GAME_API void GameRenderer::AddShadowCasterCamera(const render::Camera& camera)
	{
		allowedCamera[core::Name{ "ShadowMapPass" }].insert(&camera);
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass->passName == "ShadowMapPass")
				continue;
			ignoreCamera[pass->passName].insert(&camera);
		}
	}

	SH_GAME_API void GameRenderer::RemoveShadowCasterCamera(const render::Camera& camera)
	{
		allowedCamera[core::Name{ "ShadowMapPass" }].erase(&camera);
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass->passName == "ShadowMapPass")
				continue;
			ignoreCamera[pass->passName].erase(&camera);
		}
	}
	SH_GAME_API void GameRenderer::AddShadowPass()
	{
		shadowMapPass = &AddRenderPass<render::ShadowMapPass>();
		allowedCamera.insert({ shadowMapPass->passName, {} });
	}
}//namespace
