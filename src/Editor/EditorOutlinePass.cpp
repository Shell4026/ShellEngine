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
		std::vector<const render::Camera*> camVec{ &camera->GetNative() };

		auto beforeMask = camera->GetNative().GetRenderTagMask();
		auto beforeRenderTexture = camera->GetNative().GetRenderTexture();
		camera->GetNative().SetRenderTagMask(1);
		camera->GetNative().SetRenderTexture(output);

		render::RenderPipeline::RecordCommand(camVec, imgIdx);

		camera->GetNative().SetRenderTagMask(beforeMask);
		camera->GetNative().SetRenderTexture(beforeRenderTexture);
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