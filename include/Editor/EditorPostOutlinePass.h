#pragma once
#include "Export.h"

#include "Render/ScriptableRenderPass.h"

namespace sh::render
{
	class RenderTexture;
	class Mesh;
	class Drawable;
}

namespace sh::editor
{
	class EditorPostOutlinePass : public render::ScriptableRenderPass
	{
	public:
		SH_EDITOR_API EditorPostOutlinePass(const render::IRenderContext& ctx);
		SH_EDITOR_API ~EditorPostOutlinePass();
		SH_EDITOR_API void SetOutlineMaterial(render::Material& mat);
	protected:
		SH_EDITOR_API void Configure(const render::RenderData& renderData) override;
		SH_EDITOR_API void Record(render::CommandBuffer& cmd, const render::IRenderContext& ctx, const render::RenderData& renderData) override;
	private:
		const render::IRenderContext& ctx;

		render::Mesh* plane = nullptr;
		render::Material* mat = nullptr;

		render::Drawable* drawable = nullptr;
	};
}//namespace