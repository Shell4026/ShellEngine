#pragma once

#include "Export.h"

#include "Game/ImGUI.h"

namespace sh::editor
{
	class UI
	{
	protected:
		const game::ImGUI& imgui;
	public:
		SH_EDITOR_API UI(const game::ImGUI& imgui);

		SH_EDITOR_API virtual void Update() = 0;
	};
}