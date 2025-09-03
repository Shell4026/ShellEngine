#include "TransparentPipeline.h"

#include <algorithm>
namespace sh::render
{
	TransparentPipeline::TransparentPipeline()
	{
		passName = core::Name("Transparent");

		SetClear(false);
	}
	SH_RENDER_API void TransparentPipeline::PushDrawable(Drawable* drawable)
	{
		RenderPipeline::PushDrawable(drawable);
		bAddedDrawable = true;
	}
	SH_RENDER_API void TransparentPipeline::RecordCommand(const std::vector<const Camera*>& cameras, uint32_t imgIdx)
	{
		if (bAddedDrawable)
		{
			for (auto& renderGroup : renderGroups)
			{
				std::sort(renderGroup.drawables.begin(), renderGroup.drawables.end(),
					[](const Drawable* left, const Drawable* right) -> bool
					{
						return left->GetPriority() > right->GetPriority();
					}
				);
			}
			bAddedDrawable = false;
		}
		RenderPipeline::RecordCommand(cameras, imgIdx);
	}
}//namespace