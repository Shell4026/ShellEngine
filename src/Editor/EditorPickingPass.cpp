#include "EditorPickingPass.h"

namespace sh::editor
{
	EditorPickingPass::EditorPickingPass()
	{
		passName = core::Name("EditorPicking");
	}
	SH_EDITOR_API void EditorPickingPass::RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx)
	{
		assert(pickingCamera.IsValid());
		std::vector<const render::Camera*> camVector{};
		if (pickingCamera->GetNative().GetActive())
			camVector.push_back(&pickingCamera->GetNative());

		render::RenderPipeline::RecordCommand(camVector, imgIdx);
	}
	SH_EDITOR_API void EditorPickingPass::SetCamera(const game::Camera& pickingCamera)
	{
		this->pickingCamera = &pickingCamera;
	}
}//namespace