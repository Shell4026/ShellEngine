#include "EditorPostOutlinePass.h"

#include "Core/GarbageCollection.h"

#include "Render/Mesh.h"

namespace sh::editor
{
	EditorPostOutlinePass::EditorPostOutlinePass()
	{
		passName = core::Name("EditorPostOutline");
		SetClear(false);

		plane = core::SObject::Create<render::Mesh>();

		core::GarbageCollection::GetInstance()->SetRootSet(plane);

		plane->SetVertex({ 
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.f, 0.f, 0.f} },
			{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.f, 0.f, 0.f} },
			{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.f, 0.f, 0.f} },
			{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.f, 0.f, 0.f} },
		});
		plane->SetIndices({ 0, 1, 2, 2, 3, 0 });
	}
	EditorPostOutlinePass::~EditorPostOutlinePass()
	{
		core::GarbageCollection::GetInstance()->ForceDelete(drawable);
		core::GarbageCollection::GetInstance()->ForceDelete(plane);
	}
	SH_EDITOR_API void EditorPostOutlinePass::Create(render::IRenderContext& context)
	{
		this->context = &context;

		plane->Build(context);
		render::RenderPipeline::Create(context);
	}
	SH_EDITOR_API void EditorPostOutlinePass::RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx)
	{
		std::vector<const render::Camera*> camVec{};
		if (camera->GetNative().GetActive())
			camVec.push_back(&camera->GetNative());

		render::RenderPipeline::RecordCommand(camVec, imgIdx);
	}
	SH_EDITOR_API void EditorPostOutlinePass::ClearDrawable()
	{
		RenderPipeline::ClearDrawable();
		if (drawable != nullptr)
			RenderPipeline::PushDrawable(drawable);
	}
	SH_EDITOR_API void EditorPostOutlinePass::SetCamera(game::Camera& camera)
	{
		this->camera = &camera;
	}
	SH_EDITOR_API void EditorPostOutlinePass::SetOutlineMaterial(render::Material& mat)
	{
		this->mat = &mat;

		if (drawable != nullptr)
			drawable->Destroy();

		drawable = core::SObject::Create<render::Drawable>(mat, *plane);
		drawable->Build(*context);

		core::GarbageCollection::GetInstance()->SetRootSet(drawable);
	}
}//namespace