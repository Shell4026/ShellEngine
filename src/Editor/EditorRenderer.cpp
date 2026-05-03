#include "EditorRenderer.h"

#include "Game/GUIPass.h"

#include "Render/RenderPass/TransparentPass.h"
#include "Render/RenderPass/ShadowMapPass.h"

#include <algorithm>
namespace sh::editor
{
	EditorRenderer::EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, game::World& world) :
		GameRenderer(ctx, guictx, world)
	{
	}
	SH_EDITOR_API void EditorRenderer::Init()
	{
		pickingPass = &AddRenderPass(core::Name{ "EditorPicking" }, render::RenderQueue::Picking);
		outlinePass = &AddRenderPass<EditorOutlinePass>();
		shadowMapPass = &AddRenderPass<render::ShadowMapPass>();
		opaquePass = &AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		transparentPass = &AddRenderPass<render::TransparentPass>();
		uiPass = &AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::Transparent);
		postOutlinePass = &AddRenderPass<EditorPostOutlinePass>(ctx);
		guiPass = &AddRenderPass<game::GUIPass>();
		guiPass->SetImGUIContext(guiCtx);
	}
	SH_EDITOR_API void EditorRenderer::Setup(const render::RenderData& data)
	{
		if (data.tag == "ImGUI")
		{
			EnqueRenderPass(*guiPass);
			return;
		}
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
		if (data.tag == "ImGUI")
		{
			EnqueRenderPass(*guiPass);
			return;
		}
		if (data.tag == "Shadow")
		{
			EnqueRenderPass(*shadowMapPass);
			return;
		}
	}
}//namespace