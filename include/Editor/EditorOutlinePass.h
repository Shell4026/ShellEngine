#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"

#include "Render/RenderData.h"
#include "Render/ScriptableRenderPass.h"

#include "Game/Component/Render/Camera.h"

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
		SH_EDITOR_API void Configure(const render::RenderData& renderData) override;
		SH_EDITOR_API void Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData) override;
	private:
		render::RenderData rd;
	};
}//namespace