#pragma once
#include "Export.h"
#include "RenderPipeline.h"

namespace sh::render
{
	/// @brief 단순히 우선 순위 정렬기능만 추가한 RenderPipeline
	class TransparentPipeline : public RenderPipeline
	{
	public:
		SH_RENDER_API TransparentPipeline();

		SH_RENDER_API void PushDrawable(Drawable* drawable) override;
		SH_RENDER_API void RecordCommand(const std::vector<const Camera*>& cameras, uint32_t imgIdx) override;
	private:
		bool bAddedDrawable = false;
	};
}//namespace