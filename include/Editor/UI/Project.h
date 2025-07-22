#pragma once
#include "Export.h"

#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/Plugin.h"

#include "Game/GUITexture.h"

#include <string>
#include <filesystem>
namespace sh::editor
{
	class AssetDatabase;
	class EditorWorld;
	class Project
	{
	private:
		static constexpr const ImVec4 iconBackgroundColor{ 0, 0, 0, 0 };

		std::filesystem::path rootPath;
		std::filesystem::path assetPath;
		std::filesystem::path binaryPath;
		std::filesystem::path libraryPath;
		std::filesystem::path currentPath;
		std::filesystem::path selected;

		AssetDatabase& assetDatabase;

		std::vector<std::filesystem::path> foldersPath;
		std::vector<std::filesystem::path> filesPath;
		std::vector<std::string> invisibleExtensions;

		const game::GUITexture *folderIcon, *fileIcon, *meshIcon;
		float iconSize = 50.0f;

		core::Plugin userPlugin;
		std::vector<std::pair<std::string, const core::reflection::STypeInfo*>> userComponents;

		static bool bInitResource;
		bool isOpen = false;
	public:
		static constexpr const char* name = "Project";
		EditorWorld& world;
	private:
		void InitResources();
		void GetAllFiles(const std::filesystem::path& path);
		/// @brief 경로의 파일의 이름의 폰트 크기가 maxSize가 넘어가면 잘라내는 함수
		/// @param path 파일 경로
		/// @param maxSize 최대 길이
		/// @return 새로 만들어진 이름
		auto GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string;
		void RenderParentFolder();
		void SetDragItem(const std::filesystem::path& path);
		auto GetIcon(const std::filesystem::path& path) const -> const game::GUITexture*;
		bool RenderFile(const std::filesystem::path& path, float& cursorX, float spacing, float width);
		void ShowRightClickPopup();
		void RenderNameBar();

		void LoadUserModule();

		void CopyProjectTemplate(const std::filesystem::path& targetDir);
	public:
		SH_EDITOR_API Project(EditorWorld& world);
		SH_EDITOR_API ~Project();

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 새 프로젝트를 만드는 함수
		/// @param dir 경로
		SH_EDITOR_API void CreateNewProject(const std::filesystem::path& dir);
		SH_EDITOR_API void OpenProject(const std::filesystem::path& dir);

		SH_EDITOR_API void SaveWorld(const std::string& name);
		SH_EDITOR_API void LoadWorld(const std::string& name);

		SH_EDITOR_API auto IsProjectOpen() const -> bool;

		SH_EDITOR_API void ReloadModule();

		SH_EDITOR_API auto GetAssetPath() const -> const std::filesystem::path&;
		SH_EDITOR_API auto GetBinPath() const -> const std::filesystem::path&;
	};
}