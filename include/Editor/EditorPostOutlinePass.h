#pragma once
#include "Export.h"

#include "Render/RenderPipeline.h"

#include "Game/Component/Camera.h"

namespace sh::render
{
	class RenderTexture;
	class Mesh;
	class Drawable;
}

namespace sh::editor
{
	class EditorPostOutlinePass : public render::RenderPipeline
	{
	private:
		render::IRenderContext* context = nullptr;

		game::Camera* camera = nullptr;

		render::Mesh* plane = nullptr;
		render::Material* mat = nullptr;

		render::Drawable* drawable = nullptr;
	public:
		SH_EDITOR_API EditorPostOutlinePass();
		SH_EDITOR_API ~EditorPostOutlinePass();

		SH_EDITOR_API void Create(render::IRenderContext& context) override;
		SH_EDITOR_API void RecordCommand(const std::vector<const render::Camera*>& cameras, uint32_t imgIdx) override;
		SH_EDITOR_API void ClearDrawable() override;

		SH_EDITOR_API void SetCamera(game::Camera& camera);
		SH_EDITOR_API void SetOutlineMaterial(render::Material& mat);
	};
}//namespace