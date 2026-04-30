#include "EditorOutlinePass.h"

#include "Core/GarbageCollection.h"

#include "Render/RenderTexture.h"

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
		output = &tex;
	}

	SH_EDITOR_API void EditorOutlinePass::Configure(const render::RenderTarget& renderData)
	{
		render::RenderTarget rd;
		rd.camera = renderData.camera;
		rd.frameIndex = renderData.frameIndex;
		rd.target = output.Get();
		rd.drawables = renderData.drawables;

		ScriptableRenderPass::Configure(rd);
	}
	SH_EDITOR_API void EditorOutlinePass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderTarget& renderData)
	{
		render::RenderTarget rd;
		rd.camera = renderData.camera;
		rd.frameIndex = renderData.frameIndex;
		rd.target = output.Get();

		SetViewportScissor(cmd, ctx, rd);
		cmd.SetRenderTarget(rd, true, true, true, true);

		if (rd.drawables == nullptr)
			return;

		for (const RenderBatch& batch : renderBatches)
		{
			cmd.DrawMeshBatch(batch.drawables, passName);
		}
	}
}//namespace