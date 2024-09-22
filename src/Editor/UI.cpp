#include "UI.h"

namespace sh::editor
{
	UI::UI(game::ImGUI& imgui) :
		imgui(imgui)
	{
		ImGui::SetCurrentContext(imgui.GetContext());
	}
}