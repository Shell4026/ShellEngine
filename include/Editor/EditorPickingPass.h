#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Game/Component/Camera.h"

#include "Render/RenderPipeline.h"

namespace sh::editor
{
	class EditorPickingPass : public render::RenderPipeline
	{
	private:
		core::SObjWeakPtr<const game::Camera> pickingCamera = nullptr;
	public:
		SH_EDITOR_API EditorPickingPass();

		SH_EDITOR_API void RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx) override;

		SH_EDITOR_API void SetCamera(const game::Camera& pickingCamera);
	};
}//namespace