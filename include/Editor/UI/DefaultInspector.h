#pragma once
#include "Export.h"
#include "CustomInspector.h"

#include "Render/Material.h"
#include "Render/Texture.h"

namespace sh::editor
{
	class MaterialInspector : public CustomInspector<MaterialInspector, render::Material>
	{
	public:
		SH_EDITOR_API void RenderUI(void* obj) override;
	};

	class TextureInspector : public CustomInspector<TextureInspector, render::Texture>
	{
	public:
		SH_EDITOR_API void RenderUI(void* obj) override;
	};
}//namespace