#include "RenderPass/TransparentPass.h"
#include "Drawable.h"
#include "CommandBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

#include <algorithm>
namespace sh::render
{
	TransparentPass::TransparentPass(std::string_view name, RenderQueue renderQueue) :
		ScriptableRenderPass(core::Name(name), renderQueue)
	{
	}

	SH_RENDER_API void TransparentPass::Configure(const RenderData& renderData)
	{
		SetImageUsages(renderData);
	}
	SH_RENDER_API void TransparentPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderTarget)
	{
		cmd.SetRenderData(renderTarget, false, false, true, false);

		if (renderTarget.drawables == nullptr)
			return;

		items.clear();
		for (const Drawable* drawable : *renderTarget.drawables)
		{
			RenderItem item{};
			item.material = drawable->GetMaterial();
			item.topology = drawable->GetTopology(core::ThreadType::Render);
			item.drawable = drawable;
			items.push_back(item);
		}

		std::size_t idx = 0;
		for (const RenderViewer& viewer : renderTarget.renderViewers)
		{
			const glm::vec3& camPos = viewer.pos;
			const glm::vec3& camTo = viewer.to;
			const glm::vec3 to = glm::normalize(camTo - camPos);
			std::stable_sort(items.begin(), items.end(),
				[&camPos, &to](const RenderItem& left, const RenderItem& right) -> bool
				{
					const glm::vec3 posLeft = left.drawable->GetModelMatrix(core::ThreadType::Render)[3];
					const glm::vec3 posRight = right.drawable->GetModelMatrix(core::ThreadType::Render)[3];
					const float leftLen = glm::dot(to, posLeft - camPos);
					const float rightLen = glm::dot(to, posRight - camPos);
					return leftLen > rightLen;
				}
			);

			for (const RenderItem& item : items)
				cmd.DrawMesh(*item.drawable, passName, idx);
			++idx;
		}
	}
}//namespace