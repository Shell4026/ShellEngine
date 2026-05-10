#include "RenderPass/CombinePass.h"
#include "CommandBuffer.h"
#include "Drawable.h"
#include "IRenderContext.h"
#include "Material.h"
#include "Mesh.h"

#include "Core/GarbageCollection.h"
#include "Core/SObject.h"

namespace sh::render
{
	CombinePass::CombinePass(const IRenderContext& ctx) :
		ScriptableRenderPass(core::Name{ "CombinePass" }, RenderQueue::AfterRendering),
		ctx(ctx)
	{
		resource.plane = core::SObject::Create<Mesh>();

		resource.plane->SetVertex({
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.f, 0.f, 0.f}},
			{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.f, 0.f, 0.f}},
			{{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}, {0.f, 0.f, 0.f}},
			{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}, {0.f, 0.f, 0.f}},
		});
		resource.plane->SetIndices({ 0, 1, 2, 2, 3, 0 });
		resource.plane->Build(ctx);

		onMatDestroy.SetCallback(
			[this](const core::SObject* objPtr)
			{
				resource.mat = nullptr;
				if (resource.drawable != nullptr)
					resource.drawable->Destroy();
				resource.drawable = nullptr;
			}
		);
	}
	CombinePass::~CombinePass()
	{
		core::GarbageCollection& gc = *core::GarbageCollection::GetInstance();
		gc.ForceDelete(resource.plane);
		gc.ForceDelete(resource.drawable);
		resource.plane = nullptr;
		resource.drawable = nullptr;
		resource.mat = nullptr;
	}

	SH_RENDER_API void CombinePass::SetMaterial(Material& mat)
	{
		if (resource.mat == &mat || mat.IsPendingKill())
			return;
		resource.mat = &mat;
		resource.mat->onDestroy.Register(onMatDestroy);
		if (resource.drawable != nullptr)
			resource.drawable->SetMaterial(*resource.mat);
	}

	void CombinePass::EnsureDrawable()
	{
		if (resource.drawable != nullptr || resource.mat == nullptr)
			return;
		resource.drawable = core::SObject::Create<Drawable>(*resource.mat, *resource.plane);
		resource.drawable->Build(ctx);
	}

	SH_RENDER_API void CombinePass::Configure(const RenderData& renderData)
	{
		renderTextures.clear();

		if (!core::IsValid(resource.mat))
			return;

		EnsureDrawable();
		if (resource.drawable == nullptr || !core::IsValid(resource.drawable->GetMesh()) || !core::IsValid(resource.drawable->GetMaterial()))
			return;

		SetRenderTargetImageUsages(renderData);
		SetImageUsages(*resource.mat);
	}

	SH_RENDER_API void CombinePass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
	{
		if (!core::IsValid(resource.mat) || !core::IsValid(resource.drawable))
			return;

		cmd.SetRenderData(renderData, false, false, false, false);

		std::size_t viewerIdx = 0;
		for (const RenderViewer& viewer : renderData.renderViewers)
		{
			SetViewportScissor(cmd, ctx, viewer);
			cmd.DrawMesh(*resource.drawable, passName, viewerIdx);
			++viewerIdx;
		}
	}

	SH_RENDER_API void CombinePass::Resource::PushReferenceObjects(core::GarbageCollection& gc)
	{
		gc.PushReferenceObject(plane);
		gc.PushReferenceObject(drawable);
		gc.PushReferenceObject(mat);
	}
}//namespace
