#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Render/ScriptableRenderPass.h"

#include "Game/Component/Camera.h"

namespace sh::render
{
	class RenderTexture;
}

namespace sh::editor
{
	class EditorOutlinePass : public render::ScriptableRenderPass
	{
	public:
		SH_EDITOR_API EditorOutlinePass();
		SH_EDITOR_API ~EditorOutlinePass();

		SH_EDITOR_API void SetOutTexture(render::RenderTexture& tex);
	protected:
		SH_EDITOR_API void Configure(const render::RenderTarget& renderData) override;
		SH_EDITOR_API void Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderTarget& renderData) override;
	private:
		core::SObjWeakPtr<render::RenderTexture> output = nullptr;
	};
}//namespace