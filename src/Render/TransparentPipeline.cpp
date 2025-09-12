#include "TransparentPipeline.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

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
		std::vector<const Camera*> camVector;
		camVector.reserve(cameras.size());
		for (auto cam : cameras)
		{
			if (cam == nullptr)
				continue;
			if (auto it = std::find(ignoreCameras.begin(), ignoreCameras.end(), cam); it == ignoreCameras.end())
				camVector.push_back(cam);
		}
		if (camVector.size() == 0)
			return;

		for (auto cam : camVector)
		{
			const auto& camPos = cam->GetPos(core::ThreadType::Render);
			for (auto& renderGroup : renderGroups)
			{
				std::sort(renderGroup.drawables.begin(), renderGroup.drawables.end(),
					[camPos](const Drawable* left, const Drawable* right) -> bool
					{
						const glm::vec3 posLeft = left->GetModelMatrix(core::ThreadType::Render)[3];
						const glm::vec3 posRight = right->GetModelMatrix(core::ThreadType::Render)[3];
						float leftLenSqr = glm::distance2(posLeft, camPos);
						float rightLenSqr = glm::distance2(posRight, camPos);
						return leftLenSqr > rightLenSqr;
					}
				);
			}

			impl->RecordCommand(passName, *cam, renderGroups, imgIdx);
		}
	}
}//namespace