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
		ExplorerUI(game::ImGUImpl& imgui);

		void Update() override;
		void Render() override;
	};
}