#include "UI.h"

namespace sh::editor
{
	UI::UI(const game::ImGUI& imgui) :
		imgui(imgui)
	{
		ImGui::SetCurrentContext(imgui.GetContext());
	}
}