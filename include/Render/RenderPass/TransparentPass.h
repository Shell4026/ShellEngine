#pragma once
#include "Render/Export.h"
#include "Render/ScriptableRenderPass.h"
#include "Render/Mesh.h"

#include <vector>
namespace sh::render
{
	class Material;
	class Drawables;
	/// @brief 단순히 우선 순위 정렬기능만 추가된 패스
	class TransparentPass : public ScriptableRenderPass
	{
	public:
		SH_RENDER_API TransparentPass(std::string_view name = "Transparent", RenderQueue renderQueue = RenderQueue::Transparent);

		SH_RENDER_API void Configure(const RenderData& renderData) override;
		SH_RENDER_API void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData) override;
	private:
		struct RenderItem
		{
			const Material* material;
			Mesh::Topology topology;
			const Drawable* drawable;
		};
		std::vector<RenderItem> items;
	};
}//namespace