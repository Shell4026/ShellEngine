#pragma once
#include "Export.h"

#include "Render/ScriptableRenderPass.h"

#include <memory>
#include <functional>
namespace sh::game
{
	class ImGUImpl;

	class UIPass : public render::ScriptableRenderPass
	{
	public:
		SH_GAME_API UIPass();

		SH_GAME_API void SetImGUIContext(ImGUImpl& gui);
	protected:
		SH_GAME_API void Configure(const render::RenderTarget& renderData) override;
		SH_GAME_API auto BuildDrawList(const render::RenderTarget& renderData) -> render::DrawList override;
	public:
		render::RenderTexture* viewportTexture = nullptr;
	private:
		ImGUImpl* gui = nullptr;
	};
}//namespace