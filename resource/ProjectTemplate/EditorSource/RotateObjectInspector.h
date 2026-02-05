#pragma once
#include "ExportEditor.h"
#include "RotateObject.h"

#include "Editor/UI/CustomInspector.h"

namespace sh::editor
{
	class RotateObjectInspector : public ICustomInspector
	{
		INSPECTOR(RotateObjectInspector, game::RotateObject)
	public:
		SH_EDIT_API void RenderUI(void* obj, int idx) override;
	};
}//namespace