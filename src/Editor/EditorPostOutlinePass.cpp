#include "EditorPostOutlinePass.h"

#include "Core/GarbageCollection.h"

#include "Render/Mesh.h"
#include "Render/Material.h"
#include "Render/Drawable.h"
#include "Render/IRenderContext.h"

namespace sh::editor
{
	EditorPostOutlinePass::EditorPostOutlinePass(const render::IRenderContext& ctx) :
		ScriptableRenderPass(core::Name("EditorPostOutline"), render::RenderQueue::AfterTransparent),
		ctx(ctx)
	{
		plane = core::SObject::Create<render::Mesh>();

		core::GarbageCollection::GetInstance()->SetRootSet(plane);

		plane->SetVertex({ 
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.f, 0.f, 0.f} },
			{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.f, 0.f, 0.f} },
			{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.f, 0.f, 0.f} },
			{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.f, 0.f, 0.f} },
		});
		plane->SetIndices({ 0, 1, 2, 2, 3, 0 });
		plane->Build(ctx);
	}
	EditorPostOutlinePass::~EditorPostOutlinePass()
	{
		core::GarbageCollection::GetInstance()->ForceDelete(drawable);
		core::GarbageCollection::GetInstance()->ForceDelete(plane);
	}
	SH_EDITOR_API void EditorPostOutlinePass::SetOutlineMaterial(render::Material& mat)
	{
		this->mat = &mat;
	}

	SH_EDITOR_API void EditorPostOutlinePass::Configure(const render::RenderData& renderData)
	{
		if (drawable == nullptr && mat != nullptr)
		{
			drawable = core::SObject::Create<render::Drawable>(*mat, *plane);
			drawable->Build(ctx);

			core::GarbageCollection::GetInstance()->SetRootSet(drawable);
		}

		renderTextures.clear();
		renderTextures[renderData.target] = render::ResourceUsage::ColorAttachment;
		if (mat != nullptr)
		{
			for (auto& [name, rt] : mat->GetCachedRenderTextures())
				renderTextures[rt] = render::ResourceUsage::SampledRead;
		}
	}
	SH_EDITOR_API void EditorPostOutlinePass::Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData)
	{
		if (!core::IsValid(drawable->GetMesh()) || !core::IsValid(drawable->GetMaterial()))
			return;
		cmd.SetRenderData(renderData, false, false, false, false);
		std::size_t viewerIdx = 0;
		for (const render::RenderViewer& viewer : renderData.renderViewers)
		{
			SetViewportScissor(cmd, ctx, viewer);
			cmd.DrawMesh(*drawable, passName, viewerIdx);
			++viewerIdx;
		}
	}
}//namespace