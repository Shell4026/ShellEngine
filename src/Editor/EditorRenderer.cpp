#include "EditorRenderer.h"

#include "Game/UIPass.h"

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
		AddRenderPass(core::Name{ "EditorPicking" }, render::RenderQueue::Picking);
		outlinePass = &AddRenderPass<EditorOutlinePass>();
		AddShadowPass();
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::Transparent);
		postOutlinePass = &AddRenderPass<EditorPostOutlinePass>(ctx);
		uiPass = &AddRenderPass<game::UIPass>();
		uiPass->SetImGUIContext(guiCtx);
	}
	SH_EDITOR_API void EditorRenderer::SetImGUICamera(const render::Camera& camera)
	{
		ImGUICamera = &camera;
		allowedCamera[core::Name{ "ImGUI" }].insert(&camera);
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass->passName == "ImGUI")
				continue;
			ignoreCamera[pass->passName].insert(&camera);
		}
	}
	SH_EDITOR_API void EditorRenderer::SetEditorCamera(const render::Camera& camera)
	{
		editorCamera = &camera;
		allowedCamera[outlinePass->passName].insert(&camera);
		allowedCamera[postOutlinePass->passName].insert(&camera);

		uiPass->viewportTexture = editorCamera->GetRenderTexture();
	}
	SH_EDITOR_API void EditorRenderer::SetPickingCamera(const render::Camera& camera)
	{
		pickingCamera = &camera;
		allowedCamera[core::Name{ "EditorPicking" }].insert(&camera);
		for (const std::unique_ptr<render::ScriptableRenderPass>& pass : allPasses)
		{
			if (pass->passName == "EditorPicking")
				continue;
			ignoreCamera[pass->passName].insert(&camera);
		}
	}

	SH_EDITOR_API void EditorRenderer::Setup(const render::RenderTarget& data)
	{
		GameRenderer::Setup(data);
	}
}//namespace