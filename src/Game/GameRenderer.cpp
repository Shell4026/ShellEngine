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
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::UI);
		AddRenderPass<game::UIPass>().SetImGUIContext(guiCtx);
	}
	SH_GAME_API void GameRenderer::Setup(const render::RenderTarget& data)
	{
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			auto allowedIt = allowedCamera.find(pass->passName.ToString());
			if (allowedIt != allowedCamera.end())
			{
				const auto& allowedCams = allowedIt->second;
				bool bAllowed = std::find(allowedCams.begin(), allowedCams.end(), data.camera) != allowedCams.end();
				if (!bAllowed)
					continue;
			}

			auto it = ignoreCamera.find(pass->passName.ToString());
			if (it == ignoreCamera.end())
			{
				EnqueRenderPass(*pass);
				continue;
			}
			const auto& ignoreCams = it->second;
			bool bIgnore = std::find(ignoreCams.begin(), ignoreCams.end(), data.camera) != ignoreCams.end();
			if (bIgnore)
				continue;
			EnqueRenderPass(*pass);
		}
	}
	SH_GAME_API void GameRenderer::SetUICamera(const render::Camera& camera)
	{
		uiCamera = &camera;
		allowedCamera["UI"].push_back(&camera);
		allowedCamera["ImGUI"].push_back(&camera);
		ignoreCamera["Opaque"].push_back(&camera);
		ignoreCamera["Transparent"].push_back(&camera);
	}
}//namespace