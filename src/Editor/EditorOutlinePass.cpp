#include "EditorOutlinePass.h"

#include "Core/GarbageCollection.h"

#include "Render/RenderTexture.h"

namespace sh::editor
{
	EditorOutlinePass::EditorOutlinePass()
	{
		passName = core::Name("EditorOutline");
	}

	EditorOutlinePass::~EditorOutlinePass()
	{
	}
	SH_EDITOR_API void EditorOutlinePass::RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx)
	{
		if (camera == nullptr || output == nullptr)
			return;

		render::Camera copyCamera = camera->GetNative();

		std::vector<const render::Camera*> camVec{ &copyCamera };

		copyCamera.SetRenderTagMask(1);
		copyCamera.SetRenderTexture(output.Get());

		render::RenderPipeline::RecordCommand(camVec, imgIdx);
	}
	SH_EDITOR_API void EditorOutlinePass::SetOutTexture(render::RenderTexture& tex)
	{
		output = &tex;
	}
	SH_EDITOR_API void EditorOutlinePass::SetCamera(game::Camera& camera)
	{
		this->camera = &camera;
	}
}//namespace