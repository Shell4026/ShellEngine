#pragma once
#include "Export.h"

#include "Render/ScriptableRenderer.h"

#include "Component/Camera.h"

#include <unordered_map>
namespace sh::game
{
	class ImGUImpl;

	class GameRenderer : public render::ScriptableRenderer
	{
	public:
		SH_GAME_API GameRenderer(render::IRenderContext& ctx, game::ImGUImpl& guictx);

		SH_GAME_API void Setup(const render::RenderTarget& data) override;

		SH_GAME_API void SetUICamera(const game::Camera& camera);
	private:
		const game::Camera* uiCamera = nullptr;

		std::unordered_map<std::string, std::vector<const render::Camera*>> allowedCamera;
		std::unordered_map<std::string, std::vector<const render::Camera*>> ignoreCamera;
	};
}//namespace