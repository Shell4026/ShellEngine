#pragma once
#include "../Export.h"
#include "../ScriptableRenderPass.h"

namespace sh::render
{
	/// @brief 단순히 우선 순위 정렬기능만 추가된 패스
	class TransparentPass : public ScriptableRenderPass
	{
	public:
		SH_RENDER_API TransparentPass();

		SH_RENDER_API auto BuildDrawList(const RenderTarget& renderData) -> DrawList override;
	};
}//namespace