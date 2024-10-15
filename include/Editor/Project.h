#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/SContainer.hpp"

#include "Game/GUITexture.h"

#include <string>
#include <string_view>
#include <optional>
#include <filesystem>

namespace sh::game
{
	class World;
}
namespace sh::editor
{
	class Project : public UI
	{
	private:
		game::World& world;

		std::string dir;
		std::string currentPath;

		core::SVector<std::filesystem::path> dirs;

		game::GUITexture folderIcon;
	public:
		static constexpr const char* name = "Project";
	private:

	public:
		SH_EDITOR_API Project(game::ImGUImpl& imgui, game::World& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}