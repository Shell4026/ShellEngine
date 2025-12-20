#include "RenderPass/TransparentPass.h"
#include "Camera.h"
#include "Drawable.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <algorithm>
namespace sh::render
{
	TransparentPass::TransparentPass() :
		ScriptableRenderPass(core::Name("Transparent"), RenderQueue::Transparent)
	{
	}
	SH_RENDER_API auto TransparentPass::BuildDrawList(const RenderTarget& renderData) -> DrawList
	{
		DrawList list = ScriptableRenderPass::BuildDrawList(renderData);
		list.bClearColor = false;

		const auto& camPos = renderData.camera->GetPos(core::ThreadType::Render);
		for (auto& renderGroup : list.groups)
		{
			std::sort(renderGroup.drawables.begin(), renderGroup.drawables.end(),
				[&camPos](const Drawable* left, const Drawable* right) -> bool
				{
					const glm::vec3 posLeft = left->GetModelMatrix(core::ThreadType::Render)[3];
					const glm::vec3 posRight = right->GetModelMatrix(core::ThreadType::Render)[3];
					float leftLenSqr = glm::distance2(posLeft, camPos);
					float rightLenSqr = glm::distance2(posRight, camPos);
					return leftLenSqr > rightLenSqr;
				}
			);
		}
		return list;
	}
}//namespace