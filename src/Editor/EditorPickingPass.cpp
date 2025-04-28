#include "EditorPickingPass.h"

namespace sh::editor
{
	EditorPickingPass::EditorPickingPass()
	{
		passName = core::Name("EditorPicking");
	}
	SH_EDITOR_API void EditorPickingPass::RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx)
	{
		assert(pickingCamera);
		std::vector<const render::Camera*> camVector{};
		if (pickingCamera->GetActive())
			camVector.push_back(pickingCamera);

		render::RenderPipeline::RecordCommand(camVector, imgIdx);
	}
	SH_EDITOR_API void EditorPickingPass::SetCamera(const render::Camera& pickingCamera)
	{
		this->pickingCamera = &pickingCamera;
	}
}//namespace