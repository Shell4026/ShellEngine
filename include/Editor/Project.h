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
		static constexpr const ImVec4 iconBackgroundColor{ 0, 0, 0, 1 };
		
		std::string dir;
		std::filesystem::path rootPath;
		std::filesystem::path currentPath;

		core::SVector<std::filesystem::path> filesPath;

		game::GUITexture folderIcon, fileIcon;
		float iconSize = 50.0f;

		static bool bInitResource;
	public:
		static constexpr const char* name = "Project";
	private:
		void InitResources();
		void GetAllFiles(const std::filesystem::path& path);
		auto GetElideFileName(std::string_view name, float maxSize) const -> std::string;
		inline void RenderParentFolder();
	public:
		SH_EDITOR_API Project(game::ImGUImpl& imgui, game::World& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}