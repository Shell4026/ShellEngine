#include "RenderPass/TransparentPass.h"
#include "Camera.h"
#include "Drawable.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <algorithm>
namespace sh::render
{
	TransparentPass::TransparentPass(std::string_view name, RenderQueue renderQueue) :
		ScriptableRenderPass(core::Name(name), renderQueue)
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
		const auto& camTo = renderData.camera->GetLookPos(core::ThreadType::Render);
		const glm::vec3 to = glm::normalize(camTo - camPos);
		std::stable_sort(std::get<1>(list.renderData).begin(), std::get<1>(list.renderData).end(),
			[&camPos, &to](const DrawList::RenderItem& left, const DrawList::RenderItem& right) -> bool
			{
				const glm::vec3 posLeft = left.drawable->GetModelMatrix(core::ThreadType::Render)[3];
				const glm::vec3 posRight = right.drawable->GetModelMatrix(core::ThreadType::Render)[3];
				const float leftLen = glm::dot(to, posLeft - camPos);
				const float rightLen = glm::dot(to, posRight - camPos);
				return leftLen > rightLen;
			}
		);
		return list;
	}
}//namespace