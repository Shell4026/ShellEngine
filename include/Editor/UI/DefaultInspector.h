#pragma once
#include "Export.h"
#include "CustomInspector.h"

#include "Render/Material.h"
#include "Render/Texture.h"

#include "Game/Component/Camera.h"

namespace sh::editor
{
	class MaterialInspector : public ICustomInspector
	{
		INSPECTOR(MaterialInspector, render::Material)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class TextureInspector : public ICustomInspector
	{
		INSPECTOR(TextureInspector, render::Texture)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};

	class CameraInspector : public ICustomInspector
	{
		INSPECTOR(CameraInspector, game::Camera)
	public:
		SH_EDITOR_API void RenderUI(void* obj, int idx) override;
	};
}//namespace