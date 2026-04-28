#pragma once
#include "Export.h"
#include "EditorOutlinePass.h"
#include "EditorPostOutlinePass.h"

#include "Render/RenderPass/ShadowMapPass.h"

#include "Game/Component/Render/Camera.h"
#include "Game/GameRenderer.h"

#include <unordered_map>
namespace sh::game
{
	class ImGUImpl;
	class UIPass;
	class World;
}
namespace sh::editor
{
	class EditorRenderer : public game::GameRenderer
	{
	public:
		SH_EDITOR_API EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, game::World& world);

		SH_EDITOR_API void Init() override;

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
	};
}//namespace