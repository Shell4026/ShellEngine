#include "RenderPass/SSAOPass.h"
#include "RenderTexture.h"
#include "CommandBuffer.h"
#include "Mesh.h"
#include "Material.h"
#include "Drawable.h"
#include "IRenderContext.h"

#include "Core/SObject.h"
#include "Core/GarbageCollection.h"

namespace sh::render
{
	SSAOPass::SSAOPass(const IRenderContext& ctx) :
		ScriptableRenderPass(core::Name{ "SSAOPass" }, RenderQueue::AfterOpaque),
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
	SSAOPass::~SSAOPass() = default;

	SH_RENDER_API void SSAOPass::SetMaterial(Material& mat)
	{
		if (resource.mat == &mat || mat.IsPendingKill())
			return;
		resource.mat = &mat;
		resource.mat->onDestroy.Register(onMatDestroy);
		if (resource.drawable != nullptr)
			resource.drawable->SetMaterial(*resource.mat);
	}

	void SSAOPass::EnsureDrawable()
	{
		if (resource.drawable != nullptr || resource.mat == nullptr)
			return;
		resource.drawable = core::SObject::Create<Drawable>(*resource.mat, *resource.plane);
		resource.drawable->Build(ctx);
	}

	SH_RENDER_API void SSAOPass::Configure(const RenderData& renderData)
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
	SH_RENDER_API void SSAOPass::Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData)
	{
		if (!core::IsValid(resource.mat) || !core::IsValid(resource.drawable))
			return;
		localRenderData.SetRenderTargets(renderData.GetRenderTargets());
		localRenderData.renderViewers = renderData.renderViewers;

		cmd.SetRenderData(renderData, true, false, true, false);

		std::size_t viewerIdx = 0;
		for (const RenderViewer& viewer : localRenderData.renderViewers)
		{
			SetViewportScissor(cmd, ctx, viewer);
			cmd.DrawMesh(*resource.drawable, passName, viewerIdx);
			++viewerIdx;
		}
	}

	SH_RENDER_API void SSAOPass::Resource::PushReferenceObjects(core::GarbageCollection& gc)
	{
		gc.PushReferenceObject(plane);
		gc.PushReferenceObject(drawable);
		gc.PushReferenceObject(mat);
	}
}//namespace
