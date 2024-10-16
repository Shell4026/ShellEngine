#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/SContainer.hpp"

#include "Game/GUITexture.h"

#include <string>
#include <string_view>
#include <optional>
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

		core::SVector<std::filesystem::path> filesPath;

		game::GUITexture folderIcon, fileIcon;
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
	public:
		SH_EDITOR_API Project(game::ImGUImpl& imgui, EditorWorld& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}