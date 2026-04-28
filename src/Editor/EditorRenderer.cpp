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
		allowedCamera["ImGUI"].push_back(&camera);
		ignoreCamera["Opaque"].push_back(&camera);
		ignoreCamera["Transparent"].push_back(&camera);
		ignoreCamera["UI"].push_back(&camera);
	}
	SH_EDITOR_API void EditorRenderer::SetEditorCamera(const render::Camera& camera)
	{
		editorCamera = &camera;
		allowedCamera[outlinePass->passName].push_back(&camera);
		allowedCamera[postOutlinePass->passName].push_back(&camera);

		uiPass->viewportTexture = editorCamera->GetRenderTexture();
	}
	SH_EDITOR_API void EditorRenderer::SetPickingCamera(const render::Camera& camera)
	{
		pickingCamera = &camera;
		allowedCamera["EditorPicking"].push_back(&camera);
		ignoreCamera["Opaque"].push_back(&camera);
		ignoreCamera["Transparent"].push_back(&camera);
		ignoreCamera["ImGUI"].push_back(&camera);
		ignoreCamera["UI"].push_back(&camera);
	}

	SH_EDITOR_API void EditorRenderer::Setup(const render::RenderTarget& data)
	{
		GameRenderer::Setup(data);
	}
}//namespace