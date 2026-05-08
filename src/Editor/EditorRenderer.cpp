#include "EditorRenderer.h"

#include "Game/GUIPass.h"
#include "Render/RenderPass/ShadowMapPass.h"
#include "Render/RenderPass/TransparentPass.h"

#include <algorithm>
namespace sh::editor
{
	EditorRenderer::EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, game::World& world) :
		GameRenderer(ctx, guictx, world)
	{
	}
	SH_EDITOR_API void EditorRenderer::Init()
	{
		GameRenderer::Init();
		pickingPass = &AddRenderPass(core::Name{ "EditorPicking" }, render::RenderQueue::Picking);
		outlinePass = &AddRenderPass<EditorOutlinePass>();
		postOutlinePass = &AddRenderPass<EditorPostOutlinePass>(ctx);
	}
	SH_EDITOR_API void EditorRenderer::Setup(const render::RenderData& data)
	{
		if (data.tag == "EditorCamera")
		{
			guiPass->viewportTexture = data.target;
			EnqueRenderPass(*outlinePass);
			EnqueRenderPass(*opaquePass);
			EnqueRenderPass(*transparentPass);
			EnqueRenderPass(*uiPass);
			EnqueRenderPass(*postOutlinePass);
			return;
		}
		if (data.tag == "PickingCamera")
		{
			EnqueRenderPass(*pickingPass);
			return;
		}
		GameRenderer::Setup(data);
	}
}//namespace