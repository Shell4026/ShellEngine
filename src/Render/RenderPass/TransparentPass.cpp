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
		DrawList list{};
		list.renderData = std::vector<DrawList::RenderItem>{};
		list.bClearColor = false;
		list.bClearDepth = false;
		if (renderData.drawables == nullptr)
			return list;

		for (auto drawable : *renderData.drawables)
		{
			if (!core::IsValid(drawable) || !drawable->CheckAssetValid() || drawable->GetMaterial()->GetShader()->GetShaderPasses(passName) == nullptr)
				continue;

			DrawList::RenderItem item{};
			item.material = drawable->GetMaterial();
			item.topology = drawable->GetTopology(core::ThreadType::Render);
			item.drawable = drawable;

			std::get<1>(list.renderData).push_back(item);
		}

		const auto& camPos = renderData.camera->GetPos(core::ThreadType::Render);
		std::stable_sort(std::get<1>(list.renderData).begin(), std::get<1>(list.renderData).end(),
			[&camPos](const DrawList::RenderItem& left, const DrawList::RenderItem& right) -> bool
			{
				const glm::vec3 posLeft = left.drawable->GetModelMatrix(core::ThreadType::Render)[3];
				const glm::vec3 posRight = right.drawable->GetModelMatrix(core::ThreadType::Render)[3];
				float leftLenSqr = glm::distance2(posLeft, camPos);
				float rightLenSqr = glm::distance2(posRight, camPos);
				return leftLenSqr > rightLenSqr;
			}
		);
		return list;
	}
}//namespace