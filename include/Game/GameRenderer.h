#pragma once
#include "Export.h"

#include "Render/ScriptableRenderer.h"
#include "Render/RenderPass/ShadowMapPass.h"

#include <unordered_map>
#include <set>
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
	protected:
		SH_GAME_API void AddShadowPass();
	protected:
		render::IRenderContext& renderCtx;
		World& world;
		game::ImGUImpl& guiCtx;

		game::GUIPass* guiPass = nullptr;
		render::ShadowMapPass* shadowMapPass = nullptr;
	};
}//namespace