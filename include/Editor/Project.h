#pragma once
#include "Editor/Export.h"
#include "Editor/ProjectSetting.h"
#include "UI/ProjectExplorer.h"

#include "Core/SContainer.hpp"
#include "Core/NonCopyable.h"
#include "Core/Plugin.h"

#include "Game/ComponentLoader.h"

#include <string>
#include <filesystem>
#include <regex>
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

		SH_EDITOR_API void OpenSettingUI();
		
		SH_EDITOR_API void Build();

		SH_EDITOR_API auto GetProjectPath() const -> const std::filesystem::path& { return rootPath; }
		SH_EDITOR_API auto GetAssetPath() const -> const std::filesystem::path& { return assetPath; }
		SH_EDITOR_API auto GetBinPath() const -> const std::filesystem::path& { return binaryPath; }
		SH_EDITOR_API auto GetLibraryPath() const -> const std::filesystem::path& { return libraryPath; }
		SH_EDITOR_API auto GetProjectSetting() const -> const ProjectSetting& { return setting; }
		SH_EDITOR_API auto GetProjectSetting() -> ProjectSetting& { return setting; }

		/// @brief 해당 경로가 프로젝트 경로인지
		/// @param path 경로
		/// @return 프로젝트가 존재하면 true 아니면 false
		SH_EDITOR_API static auto IsProjectPath(const std::filesystem::path& path) -> bool;
		SH_EDITOR_API static auto GetLatestProjectPath() -> std::filesystem::path;
	private:
		void RenderNameBar();

		void CopyProjectTemplate(const std::filesystem::path& targetDir);
		void ChangeSourcePath(const std::filesystem::path& projectRootPath);
		void LoadEditorPlugin();

		static void SaveLatestProjectPath(const std::filesystem::path& path);
		static auto LoadLatestProjectPath() -> std::filesystem::path;
	public:
		render::Renderer& renderer;
		game::ImGUImpl& gui;

		core::SVector<core::SObject*> loadedAssets;
	private:
		std::filesystem::path exePath;
		std::filesystem::path rootPath;
		std::filesystem::path assetPath;
		std::filesystem::path binaryPath;
		std::filesystem::path libraryPath;
		std::filesystem::path tempPath;

		ProjectExplorer projectExplorer;
		AssetDatabase& assetDatabase;

		ProjectSetting setting;

		const std::regex engineDirRegex;

		game::ComponentLoader componentLoader;

		core::SSet<core::SObject*> loadedScriptableObjects;
		core::Observer<false, core::SObject*>::Listener onAssetImportedListener;
		bool isOpen = false;
		bool bSettingUI = false;
	};
}