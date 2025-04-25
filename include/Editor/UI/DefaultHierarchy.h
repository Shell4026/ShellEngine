#pragma once
#include "Export.h"
#include "CustomHierarchy.h"

#include "Render/Model.h"

namespace sh::editor
{
	class ModelHierarchy : public ICustomHierarchy
	{
		HIERARCHY(ModelHierarchy, render::Model)
	public:
		SH_EDITOR_API void OnHierarchyDraged(EditorWorld& world, const ImGuiPayload& payload) override;
	};
}//namespace