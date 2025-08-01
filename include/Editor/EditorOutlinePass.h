﻿#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Render/RenderPipeline.h"

#include "Game/Component/Camera.h"

namespace sh::render
{
	class RenderTexture;
}

namespace sh::editor
{
	class EditorOutlinePass : public render::RenderPipeline
	{
	private:
		core::SObjWeakPtr<render::RenderTexture> output = nullptr;
		core::SObjWeakPtr<game::Camera> camera = nullptr;
	public:
		SH_EDITOR_API EditorOutlinePass();
		SH_EDITOR_API ~EditorOutlinePass();

		SH_EDITOR_API void RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx) override;

		SH_EDITOR_API void SetOutTexture(render::RenderTexture& tex);
		SH_EDITOR_API void SetCamera(game::Camera& camera);
	};
}//namespace