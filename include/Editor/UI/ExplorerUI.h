#pragma once
#include "Export.h"

#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <filesystem>
#include <initializer_list>
namespace sh::editor
{
	class EditorWorld;

	class ExplorerUI
	{
	public:
		enum class OpenMode
		{
			Select,
			Create
		};
		enum FlagEnum
		{
			None = 0,
			FolderOnly = 1,
		};
		using Flag = uint32_t;
	public:
		SH_EDITOR_API ExplorerUI();

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		SH_EDITOR_API void Open(OpenMode mode = OpenMode::Select, Flag flag = FlagEnum::None);

		SH_EDITOR_API void SetCurrentPath(const std::filesystem::path& path);
		/// @brief 해당 확장자의 파일만 보여지게 된다.
		/// @brief 사용법: SetExtensionFilter(".exe");
		/// @param extension 확장자
		SH_EDITOR_API void SetExtensionFilter(const std::string& extension);
		/// @brief 해당 확장자의 파일만 보여지게 된다.
		/// @brief 사용법: SetExtensionFilter({ ".exe", ".txt" });
		/// @param extension 확장자
		SH_EDITOR_API void SetExtensionFilter(const std::initializer_list<std::string>& extensions);
		/// @brief 확장자 필터를 초기화 한다.
		SH_EDITOR_API void ClearExtensionFilter();

		/// @brief 경로 지정/파일 선택 시 실행 할 콜백 함수를 등록하는 함수
		/// @brief 콜백 함수를 큐에 집어넣고 호출 시 제거한다.
		/// @param func 콜백 함수
		SH_EDITOR_API void PushCallbackQueue(const std::function<void(std::filesystem::path dir)>& func);
	private:
		void UpdateDirectoryEntries();
		void DrawFolderIcon();
		void RenderLeftSide();
		void RenderRightSide();

		void SelectFile();
	private:
		std::filesystem::path currentPath;
		std::vector<std::filesystem::path> folders;
		std::vector<std::filesystem::path> files;
		std::queue<std::function<void(std::filesystem::path dir)>> callbacks;

		OpenMode mode = OpenMode::Select;

		std::filesystem::path selected;

		Flag flag = FlagEnum::None;

		std::vector<std::string> extensionFilters;

		bool bOpen = false;
	};
}