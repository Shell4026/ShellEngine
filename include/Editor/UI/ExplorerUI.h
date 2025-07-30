#pragma once
#include "Export.h"

#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <filesystem>
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
	private:
		std::filesystem::path currentPath;
		std::vector<std::filesystem::path> folders;
		std::vector<std::filesystem::path> files;
		std::queue<std::function<void(std::filesystem::path dir)>> callbacks;

		OpenMode mode = OpenMode::Select;

		std::filesystem::path selected;

		bool bOpen = false;
	private:
		void UpdateDirectoryEntries();
		void DrawFolderIcon();
		void RenderLeftSide();
		void RenderRightSide();

		void SelectFile();
	public:
		SH_EDITOR_API ExplorerUI();

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		SH_EDITOR_API void Open(OpenMode mode = OpenMode::Select);

		SH_EDITOR_API void SetCurrentPath(const std::filesystem::path& path);
		/// @brief 경로 지정/파일 선택 시 실행 할 콜백 함수를 등록하는 함수
		/// @brief 콜백 함수를 큐에 집어넣고 호출 시 제거한다.
		/// @param func 콜백 함수
		SH_EDITOR_API void PushCallbackQueue(const std::function<void(std::filesystem::path dir)>& func);
	};
}