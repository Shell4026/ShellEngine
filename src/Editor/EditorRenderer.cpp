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
		AddRenderPass<render::TransparentPass>("UI", render::RenderQueue::UI);
		uiPass = &AddRenderPass<game::UIPass>();
		uiPass->SetImGUIContext(guictx);
	}
	SH_EDITOR_API void EditorRenderer::SetImGUICamera(const render::Camera& camera)
	{
		ImGUICamera = &camera;
		allowedCamera["ImGUI"].push_back(&camera);
		ignoreCamera["Opaque"].push_back(&camera);
		ignoreCamera["Transparent"].push_back(&camera);
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
		// allowedCamera에 등록된 패스는 등록된 카메라에서만 실행됨
		// ignoreCamera에 등록된 패스는 등록된 카메라에서는 실행 안 함
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