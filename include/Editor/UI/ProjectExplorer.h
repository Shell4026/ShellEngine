#pragma once
#include "Editor/Export.h"

#include "Game/ImGUImpl.h"

#include <vector>
#include <filesystem>
#include <string>
namespace sh::game
{
	class GUITexture;
}
namespace sh::editor
{
	class ProjectExplorer
	{
	public:
		SH_EDITOR_API ProjectExplorer();

		SH_EDITOR_API void SetRoot(const std::filesystem::path& root);
		SH_EDITOR_API void Refresh();

		SH_EDITOR_API void Render(float availableWidth);

		SH_EDITOR_API auto GetCurrentPath() const -> const std::filesystem::path&;
		SH_EDITOR_API void SetCurrentPath(const std::filesystem::path& p);

		SH_EDITOR_API void SetSelected(const std::filesystem::path& path);
		SH_EDITOR_API void ResetSelected();
		SH_EDITOR_API auto GetSelected() const -> const std::filesystem::path&;
	private:
		struct FileItem
		{
			std::filesystem::path path;
			game::GUITexture* icon = nullptr;
			bool isDirectory = false;
		};

		auto GetIcon(const std::filesystem::path& path) const -> game::GUITexture*;
		/// @brief 경로의 파일의 이름의 폰트 크기가 maxSize가 넘어가면 잘라내는 함수
		/// @param path 파일 경로
		/// @param maxSize 최대 길이
		/// @return 새로 만들어진 이름
		auto GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string;
		auto IsInvisibleExtension(const std::string& ext) const -> bool;

		auto RenderItem(const FileItem& fi, float& cursorX, float spacing, float width) -> bool;
		void RenderParent();
		void RenderRightClickPopup();

		void SetItemDragTarget(const std::filesystem::path& path);
		void SetFolderDragTarget(const std::filesystem::path& folderPath);

		void OnItemClicked(const std::filesystem::path& path);

		template<typename T>
		static void sortList(std::vector<T>& v)
		{
			std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.path < b.path; });
		}
	private:
		std::filesystem::path rootPath;
		std::filesystem::path currentPath;
		std::filesystem::path selected;
		std::vector<FileItem> folders;
		std::vector<FileItem> files;
		std::vector<std::string> invisibleExtensions;

		static constexpr const ImVec4 iconBackgroundColor{ 0, 0, 0, 0 };
		float iconSize = 50.0f;

		bool bChangeFolderState = false;
	};
}//namespace