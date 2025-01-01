#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/SContainer.hpp"

#include "Game/GUITexture.h"

#include <string>
#include <filesystem>

namespace sh::editor
{
	class EditorWorld;
	class Project : public UI
	{
	private:
		EditorWorld& world;
		static constexpr const ImVec4 iconBackgroundColor{ 0, 0, 0, 0 };
		
		std::string dir;
		std::filesystem::path rootPath;
		std::filesystem::path currentPath;

		core::SVector<std::filesystem::path> foldersPath;
		core::SVector<std::filesystem::path> filesPath;
		core::SVector<std::string> invisibleExtensions;

		const game::GUITexture *folderIcon, *fileIcon, *meshIcon;
		float iconSize = 50.0f;

		static bool bInitResource;
	public:
		static constexpr const char* name = "Project";
	private:
		void InitResources();
		void GetAllFiles(const std::filesystem::path& path);
		/// @brief 경로의 파일의 이름의 폰트 크기가 maxSize가 넘어가면 잘라내는 함수
		/// @param path 파일 경로
		/// @param maxSize 최대 길이
		/// @return 새로 만들어진 이름
		auto GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string;
		inline void RenderParentFolder();
		inline void SetDragItem(const std::filesystem::path& path);
		inline auto GetIcon(const std::filesystem::path& path) const -> const game::GUITexture*;
		inline bool RenderFile(const std::filesystem::path& path, float& cursorX, float spacing, float width);
		inline void ShowRightClickPopup();
	public:
		SH_EDITOR_API Project(game::ImGUImpl& imgui, EditorWorld& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;

		/// @brief 새 프로젝트를 만드는 함수
		/// @param dir 경로
		SH_EDITOR_API void CreateNewProject(const std::filesystem::path& dir);
		SH_EDITOR_API void OpenProject(const std::filesystem::path& dir);

		SH_EDITOR_API void SaveWorld();
		SH_EDITOR_API void LoadWorld();
	};
}