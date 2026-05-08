#include "GameRenderer.h"
#include "GUIPass.h"
#include "World.h"

#include "Render/RenderPass/DepthPass.h"
#include "Render/RenderPass/TransparentPass.h"

#include <cstring>
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
		depthPass = &AddRenderPass<render::DepthPass>();
		opaquePass = &AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		transparentPass = &AddRenderPass<render::TransparentPass>();
		uiPass = &AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::Transparent);
		guiPass = &AddRenderPass<game::GUIPass>();
		guiPass->SetImGUIContext(guiCtx);
	}
	SH_GAME_API void GameRenderer::Setup(const render::RenderData& data)
	{
		if (data.tag == "Depth")
		{
			EnqueRenderPass(*depthPass);
			return;
		}
		if (data.tag == "ImGUI")
		{
			if (guiPass != nullptr)
				EnqueRenderPass(*guiPass);
			return;
		}
		// 그 외 카메라
		EnqueRenderPass(*opaquePass);
		EnqueRenderPass(*transparentPass);
		EnqueRenderPass(*uiPass);
	}
}//namespace
