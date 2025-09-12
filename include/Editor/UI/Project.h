#pragma once
#include "Export.h"
#include "ProjectSetting.h"
#include "ProjectExplorer.h"

#include "Core/SContainer.hpp"
#include "Core/NonCopyable.h"

#include "Game/ResourceManager.hpp"

#include <string>
#include <filesystem>
namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class ImGUImpl;
}
namespace sh::editor
{
	class AssetDatabase;
	class Project : public core::INonCopyable
	{
	public:
		static constexpr const char* name = "Project";
	public:
		SH_EDITOR_API Project(render::Renderer& renderer, game::ImGUImpl& gui);
		SH_EDITOR_API ~Project();

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 새 프로젝트를 만드는 함수
		/// @param dir 경로
		SH_EDITOR_API void CreateNewProject(const std::filesystem::path& dir);
		SH_EDITOR_API void OpenProject(const std::filesystem::path& dir);

		SH_EDITOR_API void NewWorld(const std::string& name);
		SH_EDITOR_API void SaveWorld();
		SH_EDITOR_API void SaveAsWorld(const std::filesystem::path& worldAssetPath);
		SH_EDITOR_API void LoadWorld(const std::filesystem::path& worldAssetPath);

		SH_EDITOR_API auto IsProjectOpen() const -> bool;

		SH_EDITOR_API void ReloadModule();

		SH_EDITOR_API auto GetProjectPath() const -> const std::filesystem::path&;
		SH_EDITOR_API auto GetAssetPath() const -> const std::filesystem::path&;
		SH_EDITOR_API auto GetBinPath() const -> const std::filesystem::path&;
		SH_EDITOR_API auto GetLibraryPath() const -> const std::filesystem::path&;

		SH_EDITOR_API auto GetProjectSetting() const -> ProjectSetting&;

		SH_EDITOR_API void OpenSettingUI();
		
		SH_EDITOR_API void Build();

		SH_EDITOR_API static auto GetLatestProjectPath() -> std::filesystem::path;
	private:
		void RenderNameBar();

		void CopyProjectTemplate(const std::filesystem::path& targetDir);

		static void SaveLatestProjectPath(const std::filesystem::path& path);
		static auto LoadLatestProjectPath() -> std::filesystem::path;
	public:
		render::Renderer& renderer;
		game::ImGUImpl& gui;

		game::ResourceManager<core::SObject, core::UUID> loadedAssets;
	private:
		std::filesystem::path rootPath;
		std::filesystem::path assetPath;
		std::filesystem::path binaryPath;
		std::filesystem::path libraryPath;
		std::filesystem::path tempPath;

		ProjectExplorer projectExplorer;
		AssetDatabase& assetDatabase;

		ProjectSetting setting;

		bool isOpen = false;
		bool bSettingUI = false;
	};
}