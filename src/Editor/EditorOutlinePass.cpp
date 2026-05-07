#include "EditorOutlinePass.h"

#include "Core/GarbageCollection.h"

#include "Render/RenderTexture.h"
#include "Render/CommandBuffer.h"
namespace sh::editor
{
	EditorOutlinePass::EditorOutlinePass() :
		ScriptableRenderPass(core::Name("EditorOutline"), render::RenderQueue::BeforeOpaque)
	{
	}
	EditorOutlinePass::~EditorOutlinePass()
	{
	}
	SH_EDITOR_API void EditorOutlinePass::SetOutTexture(render::RenderTexture& tex)
	{
		rd.target = &tex;
		rd.renderViewers.resize(1);
	}

	SH_EDITOR_API void EditorOutlinePass::Configure(const render::RenderData& renderData)
	{
		renderTextures.clear();
		renderTextures[rd.target] = render::ResourceUsage::ColorAttachment;
		if (renderData.drawables != nullptr)
		{
			SetImageUsages(*renderData.drawables);
			renderBatches = CreateRenderBatch(*renderData.drawables);
		}
	}
	SH_EDITOR_API void EditorOutlinePass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData)
	{
		if (renderData.renderViewers.empty())
			return;
		rd.renderViewers[0] = renderData.renderViewers.front();
		rd.drawables = renderData.drawables;

		cmd.SetRenderData(rd, true, true, true, true);
		cmd.SetViewport(0, 0, rd.target->GetSize().x, rd.target->GetSize().y);
		cmd.SetScissor(0, 0, rd.target->GetSize().x, rd.target->GetSize().y);

		if (rd.drawables == nullptr)
			return;

		for (const RenderBatch& batch : renderBatches)
		{
			cmd.DrawMeshBatch(batch.drawables, passName);
		}
	}
}//namespace