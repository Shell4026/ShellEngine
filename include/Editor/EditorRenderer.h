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
	class GUIPass;
	class World;
}
namespace sh::render
{
	class TransparentPass;
}
namespace sh::editor
{
	class EditorRenderer : public game::GameRenderer
	{
	public:
		SH_EDITOR_API EditorRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, game::World& world);

		SH_EDITOR_API void Init() override;

		auto GetOutlinePass() const -> EditorOutlinePass* { return outlinePass; }
		auto GetPostOutlinePass() const -> EditorPostOutlinePass* { return postOutlinePass; }
	protected:
		SH_EDITOR_API void Setup(const render::RenderData& data) override;
	private:
		render::ScriptableRenderPass* pickingPass = nullptr;
		render::ScriptableRenderPass* opaquePass = nullptr;
		render::TransparentPass* transparentPass = nullptr;
		render::TransparentPass* uiPass = nullptr;
		EditorOutlinePass* outlinePass = nullptr;
		EditorPostOutlinePass* postOutlinePass = nullptr;
	};
}//namespace