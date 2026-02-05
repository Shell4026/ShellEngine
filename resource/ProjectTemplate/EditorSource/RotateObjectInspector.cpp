#include "RotateObjectInspector.h"

#include "Editor/UI/Inspector.h"

#include "Game/ImGUImpl.h"
#include "Game/World.h"
namespace sh::editor
{
	SH_EDIT_API void RotateObjectInspector::RenderUI(void* obj, int idx)
	{
		game::RotateObject* rotateObjPtr = reinterpret_cast<game::RotateObject*>(obj);
		ImGui::SetCurrentContext(rotateObjPtr->gameObject.world.GetUiContext().GetContext());
		ImGui::Text("This is RotateObject!!!");
		Inspector::RenderProperties(rotateObjPtr->GetType(), *rotateObjPtr, idx);
	}
}//namespace