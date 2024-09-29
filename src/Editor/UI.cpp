#include "UI.h"

namespace sh::editor
{
	UI::UI(game::ImGUImpl& imgui) :
		imgui(imgui)
	{
		ImGui::SetCurrentContext(imgui.GetContext());
	}
}