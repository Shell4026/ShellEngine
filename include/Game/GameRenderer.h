#pragma once
#include "Export.h"

#include "Render/ScriptableRenderer.h"
#include "Render/RenderPass/ShadowMapPass.h"

#include "Component/Render/Camera.h"

#include <unordered_map>
#include <set>
namespace sh::game
{
	class ImGUImpl;
	class World;

	class GameRenderer : public render::ScriptableRenderer
	{
	public:
		SH_GAME_API GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx, World& world);

		SH_GAME_API void Init() override;
		SH_GAME_API void Setup(const render::RenderTarget& data) override;

		SH_GAME_API void SetUICamera(const render::Camera& camera);
		SH_GAME_API void AddShadowCasterCamera(const render::Camera& camera);
		SH_GAME_API void RemoveShadowCasterCamera(const render::Camera& camera);
	protected:
		SH_GAME_API void AddShadowPass();
	protected:
		render::IRenderContext& renderCtx;
		World& world;
		game::ImGUImpl& guiCtx;

		render::ShadowMapPass* shadowMapPass = nullptr;
		std::unordered_map<core::Name, std::set<const render::Camera*>> allowedCamera;
		std::unordered_map<core::Name, std::set<const render::Camera*>> ignoreCamera;
	private:
		const render::Camera* uiCamera = nullptr;
	};
}//namespace