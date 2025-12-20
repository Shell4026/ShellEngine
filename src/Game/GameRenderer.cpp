#include "GameRenderer.h"
#include "UIPass.h"

#include "Render/RenderPass/TransparentPass.h"
namespace sh::game
{
	GameRenderer::GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx) :
		ScriptableRenderer(ctx)
	{
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		AddRenderPass<game::UIPass>().SetImGUIContext(guictx);
	}
	SH_GAME_API void GameRenderer::Setup(const render::RenderTarget& data)
	{
		for (auto& pass : allPasses)
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
	SH_GAME_API void GameRenderer::SetUICamera(const game::Camera& camera)
	{
		uiCamera = &camera;
		allowedCamera["UI"].push_back(&camera.GetNative());
		ignoreCamera["Opaque"].push_back(&camera.GetNative());
		ignoreCamera["Transparent"].push_back(&camera.GetNative());
	}
}//namespace