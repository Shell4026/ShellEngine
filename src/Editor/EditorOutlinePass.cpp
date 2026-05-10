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
		rd.SetRenderTarget(&tex);
		rd.renderViewers.resize(1);
	}

	SH_EDITOR_API void EditorOutlinePass::Configure(const render::RenderData& renderData)
	{
		renderTextures.clear();
		renderTextures[rd.GetRenderTargets()[0]] = render::ResourceUsage::ColorAttachment;
		if (renderData.GetDrawablesPtr() != nullptr)
		{
			SetImageUsages(*renderData.GetDrawablesPtr());
			renderBatches = CreateRenderBatch(passName, *renderData.GetDrawablesPtr());
		}
	}
	SH_EDITOR_API void EditorOutlinePass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData)
	{
		if (renderData.renderViewers.empty() || renderData.GetDrawablesPtr() == nullptr)
			return;

		rd.renderViewers[0] = renderData.renderViewers.front();
		render::IRenderThrMethod<render::RenderData>::SetDrawablesPtr(rd, renderData.GetDrawablesPtr());

		const render::RenderTexture& mainTaret = *rd.GetRenderTargets()[0];
		cmd.SetRenderData(rd, true, true, true, true);
		cmd.SetViewport(0, 0, mainTaret.GetSize().x, mainTaret.GetSize().y);
		cmd.SetScissor(0, 0, mainTaret.GetSize().x, mainTaret.GetSize().y);

		for (const RenderBatch& batch : renderBatches)
			cmd.DrawMeshBatch(batch.drawables, passName);
	}
}//namespace
