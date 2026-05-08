#pragma once
#include "Export.h"

#include "Render/ScriptableRenderer.h"

#include <unordered_map>
#include <set>
namespace sh::render
{
	class ShadowMapPass;
	class OpaquePass;
	class TransparentPass;
}
namespace sh::game
{
	class ImGUImpl;
	class World;
	class GUIPass;
	
	class GameRenderer : public render::ScriptableRenderer
	{
	public:
		SH_GAME_API GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, World& world);

		SH_GAME_API void Init() override;
		SH_GAME_API void Setup(const render::RenderData& data) override;

		auto GetShadowMapPass() const -> render::ShadowMapPass& { return *shadowMapPass; }
		auto GetOpaquePass() const -> render::ScriptableRenderPass& { return *opaquePass; }
		auto GetTransparentPass() const -> render::TransparentPass& { return *transparentPass; }
		auto GetUIPass() const -> render::TransparentPass& { return *uiPass; }
		auto GetGUIPass() const -> game::GUIPass& { return *guiPass; }
	protected:
		render::IRenderContext& renderCtx;
		World& world;
		game::ImGUImpl& guiCtx;

		render::ShadowMapPass* shadowMapPass = nullptr;
		render::ScriptableRenderPass* opaquePass = nullptr;
		render::TransparentPass* transparentPass = nullptr;
		render::TransparentPass* uiPass = nullptr;
		game::GUIPass* guiPass = nullptr;
	};
}//namespace