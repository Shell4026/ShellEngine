#pragma once
#include "Export.h"

#include "Render/RenderPipeline.h"
#include "Render/Camera.h"

namespace sh::editor
{
	class EditorPickingPass : public render::RenderPipeline
	{
	private:
		const render::Camera* pickingCamera = nullptr;
	public:
		SH_EDITOR_API EditorPickingPass();

		SH_EDITOR_API void RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx) override;

		SH_EDITOR_API void SetCamera(const render::Camera& pickingCamera);
	};
}//namespace