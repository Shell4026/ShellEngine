#include "EditorRenderer.h"

#include "Game/UIPass.h"

#include "Render/RenderPass/TransparentPass.h"

#include <algorithm>
namespace sh::editor
{
	EditorRenderer::EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx) :
		ScriptableRenderer(ctx)
	{
		AddRenderPass(core::Name{ "EditorPicking" }, render::RenderQueue::Picking);
		outlinePass = &AddRenderPass<EditorOutlinePass>();
		AddRenderPass(core::Name{ "Opaque" }, render::RenderQueue::Opaque);
		AddRenderPass<render::TransparentPass>();
		postOutlinePass = &AddRenderPass<EditorPostOutlinePass>(ctx);
		uiPass = &AddRenderPass<game::UIPass>();
		uiPass->SetImGUIContext(guictx);
	}
	SH_EDITOR_API void EditorRenderer::SetUICamera(const game::Camera& camera)
	{
		uiCamera = &camera;
		allowedCamera["UI"].push_back(&camera.GetNative());
		ignoreCamera["Opaque"].push_back(&camera.GetNative());
		ignoreCamera["Transparent"].push_back(&camera.GetNative());
	}
	SH_EDITOR_API void EditorRenderer::SetEditorCamera(const game::Camera& camera)
	{
		editorCamera = &camera;
		allowedCamera[outlinePass->passName].push_back(&camera.GetNative());
		allowedCamera[postOutlinePass->passName].push_back(&camera.GetNative());

		uiPass->viewportTexture = editorCamera->GetRenderTexture();
	}
	SH_EDITOR_API void EditorRenderer::SetPickingCamera(const game::Camera& camera)
	{
		pickingCamera = &camera;
		allowedCamera["EditorPicking"].push_back(&camera.GetNative());
		ignoreCamera["Opaque"].push_back(&camera.GetNative());
		ignoreCamera["Transparent"].push_back(&camera.GetNative());
	}

	SH_EDITOR_API void EditorRenderer::Setup(const render::RenderTarget& data)
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
}//namespace