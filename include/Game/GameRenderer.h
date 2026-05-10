#pragma once
#include "Export.h"

#include "Render/ScriptableRenderer.h"

#include <unordered_map>
#include <set>
namespace sh::render
{
	class DepthPass;
	class OpaquePass;
	class SSAOPass;
	class CombinePass;
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

		auto GetDepthPass() const -> render::DepthPass& { return *depthPass; }
		auto GetOpaquePass() const -> render::ScriptableRenderPass& { return *opaquePass; }
		auto GetSSAOPass() const -> render::SSAOPass& { return *ssaoPass; }
		auto GetCombinePass() const -> render::CombinePass& { return *combinePass; }
		auto GetTransparentPass() const -> render::TransparentPass& { return *transparentPass; }
		auto GetUIPass() const -> render::TransparentPass& { return *uiPass; }
		auto GetGUIPass() const -> game::GUIPass& { return *guiPass; }
	protected:
		render::IRenderContext& renderCtx;
		World& world;
		game::ImGUImpl& guiCtx;

		render::DepthPass* depthPass = nullptr;
		render::ScriptableRenderPass* opaquePass = nullptr;
		render::SSAOPass* ssaoPass = nullptr;
		render::CombinePass* combinePass = nullptr;
		render::TransparentPass* transparentPass = nullptr;
		render::TransparentPass* uiPass = nullptr;
		game::GUIPass* guiPass = nullptr;
	};
}//namespace
