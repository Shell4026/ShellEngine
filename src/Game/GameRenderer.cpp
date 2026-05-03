#include "GameRenderer.h"
#include "GUIPass.h"
#include "World.h"

#include "Render/RenderPass/TransparentPass.h"
#include "Render/ShadowMapManager.h"

#include <cstring>
namespace sh::game
{
	GameRenderer::GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, World& world) :
		ScriptableRenderer(ctx),
		renderCtx(ctx),
		world(world),
		guiCtx(guictx)
	{
		//render::ShadowMapManager::GetInstance()->Init(ctx);
	}
	SH_GAME_API void GameRenderer::Init()
	{
		//AddShadowPass();
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::UI);
		guiPass = &AddRenderPass<game::GUIPass>();
		guiPass->SetImGUIContext(guiCtx);
	}
	SH_GAME_API void GameRenderer::Setup(const render::RenderData& data)
	{
		if (data.tag == "ImGUI")
		{
			if (guiPass != nullptr)
				EnqueRenderPass(*guiPass);
			return;
		}

		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass.get() == guiPass)
				continue;
			EnqueRenderPass(*pass);
		}
	}
}//namespace
