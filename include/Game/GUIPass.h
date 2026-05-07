#pragma once
#include "Export.h"

#include "Render/ScriptableRenderPass.h"

#include <memory>
#include <functional>
namespace sh::game
{
	class ImGUImpl;

	/// @brief ImGUI전용 패스
	class GUIPass : public render::ScriptableRenderPass
	{
	public:
		SH_GAME_API GUIPass();

		SH_GAME_API void SetImGUIContext(ImGUImpl& gui);
	protected:
		SH_GAME_API void Configure(const render::RenderData& renderData) override;
		SH_GAME_API void Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData) override;
	public:
		const render::RenderTexture* viewportTexture = nullptr;
	private:
		ImGUImpl* gui = nullptr;
	};
}//namespace