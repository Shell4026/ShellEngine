#pragma once

#include "UI.h"

#include <string>
#include <vector>
namespace sh::editor
{
	class ExplorerUI : public UI
	{
	private:
		std::string currentPath;
		std::vector<std::string> directoryEntries;
	private:
		void UpdateDirectoryEntries();
	public:
		ExplorerUI(game::ImGUI& imgui);

		void Update() override;
	};
}