#pragma once
#include "Export.h"
#include "EditorOutlinePass.h"
#include "EditorPostOutlinePass.h"

#include "Render/ScriptableRenderer.h"

#include "Game/Component/Camera.h"

#include <unordered_map>
namespace sh::game
{
	class ImGUImpl;
	class UIPass;
}
namespace sh::editor
{
	class EditorRenderer : public render::ScriptableRenderer
	{
	public:
		SH_EDITOR_API EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx);

		SH_EDITOR_API auto GetOutlinePass() const -> EditorOutlinePass* { return outlinePass; }
		SH_EDITOR_API auto GetPostOutlinePass() const -> EditorPostOutlinePass* { return postOutlinePass; }

		SH_EDITOR_API void SetImGUICamera(const render::Camera& camera);
		SH_EDITOR_API void SetEditorCamera(const render::Camera& camera);
		SH_EDITOR_API void SetPickingCamera(const render::Camera& camera);
	protected:
		SH_EDITOR_API void Setup(const render::RenderTarget& data) override;
	private:
		const render::Camera* editorCamera = nullptr;
		const render::Camera* ImGUICamera = nullptr;
		const render::Camera* pickingCamera = nullptr;

		EditorOutlinePass* outlinePass = nullptr;
		EditorPostOutlinePass* postOutlinePass = nullptr;
		game::UIPass* uiPass = nullptr;
		//render::TransparentPipeline* transParentPass = nullptr;

		std::unordered_map<std::string, std::vector<const render::Camera*>> allowedCamera;
		std::unordered_map<std::string, std::vector<const render::Camera*>> ignoreCamera;
	};
}//namespace