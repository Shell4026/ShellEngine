#pragma once
#include "../Export.h"
#include "../ScriptableRenderPass.h"

#include <cstdint>
namespace sh::render
{
	/// @brief 그림자 아틀라스에 광원의 깊이 정보를 기록하는 패스
	/// @brief RenderData의 태그가 Depth면 작동
	class DepthPass : public ScriptableRenderPass
	{
	public:
		SH_RENDER_API DepthPass();

		SH_RENDER_API void Configure(const RenderData& renderData) override;
		SH_RENDER_API void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData) override;
	private:
		uint32_t lastFrameIndex = ~0u;
		uint32_t callsThisFrame = 0;
	};
}//namespace
