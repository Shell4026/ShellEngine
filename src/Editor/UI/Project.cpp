#include "UI/Project.h"
#include "EditorWorld.h"
#include "AssetDatabase.h"
#include "BuildSystem.h"
#include "UI/ProjectSettingUI.h"

#include "Core/FileSystem.h"
#include "Core/GarbageCollection.h"
#include "Core/ModuleLoader.h"

#include "Render/Renderer.h"

#include "Game/GameObject.h"
#include "Game/GameManager.h"
#include "Game/Prefab.h"
#include "Game/World.h"
namespace sh::editor
{
	Project::Project(render::Renderer& renderer, game::ImGUImpl& gui) :
		renderer(renderer), gui(gui),

		loadedAssets(renderer),
		rootPath(std::filesystem::current_path()),
		assetDatabase(*AssetDatabase::GetInstance()),
		engineDirRegex("set\\(ENGINE_DIR .*?\\)", std::regex::optimize)
	{
		exePath = core::FileSystem::GetExecutableDirectory();
		LoadLatestProjectPath();
	}

	Project::~Project()
	{
		core::ModuleLoader loader{};
		loader.Clean(editorPlugin);

		setting.Save(rootPath / "ProjectSetting.json");
		assetDatabase.SaveDatabase(libraryPath / "AssetDB.json");

		loadedAssets.Clean();
	}

	SH_EDITOR_API void Project::Update()
	{
	}
	SH_EDITOR_API void Project::Render()
	{
		static ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Project", nullptr, style);
		float h = ImGui::GetWindowContentRegionMax().y - 50;

		ImGui::BeginChild("Explorer", ImVec2{ 0.f, h }, ImGuiChildFlags_::ImGuiChildFlags_None, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);

		const ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
		const ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
		const ImVec2 windowPos = ImGui::GetWindowPos(); // 스크린 좌표

		const ImVec2 spaceMin = ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
		const ImVec2 spaceMax = ImVec2(windowPos.x + contentMax.x, windowPos.y + contentMax.y);
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const ImVec2 emptySize = ImVec2(spaceMax.x - spaceMin.x, spaceMax.y - spaceMin.y);

		const ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::SetNextItemAllowOverlap();
		if (ImGui::InvisibleButton("ProjectEmptySpace", emptySize))
		{
			projectExplorer.ResetSelected();

			for (auto& [uuid, worldPtr] : game::GameManager::GetInstance()->GetWorlds())
			{
				auto& world = static_cast<EditorWorld&>(*worldPtr);
				world.ClearSelectedObjects();
			}
		}
		// 프리팹 드래그
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ core::reflection::GetType<game::GameObject>().name }.c_str());
			if (payload != nullptr && payload->Data != nullptr)
			{
				game::GameObject* objPtr = *reinterpret_cast<game::GameObject**>(payload->Data);
				if (core::IsValid(objPtr))
				{
					auto prefab = game::Prefab::CreatePrefab(*objPtr);
					const auto exportDir = projectExplorer.GetCurrentPath() / fmt::format("{}.prefab", prefab->GetName().ToString());
					if (std::filesystem::exists(exportDir))
					{
						auto opt = assetDatabase.GetAssetUUID(exportDir);
						assert(opt.has_value());
						if (opt.has_value())
						{
							auto sobjPtr = core::SObjectManager::GetInstance()->GetSObject(opt.value());
							auto oldPrefabPtr = static_cast<game::Prefab*>(sobjPtr);
							*oldPrefabPtr = std::move(*prefab);
							assetDatabase.SetDirty(oldPrefabPtr);
							assetDatabase.SaveAllAssets();
						}
					}
					else
					{
						assetDatabase.CreateAsset(exportDir, *prefab);
						projectExplorer.Refresh();
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SetCursorPos(cursorPos);
		projectExplorer.Render(availableWidth);

		ImGui::EndChild();
		ImGui::PopStyleVar();

		RenderNameBar();
		ImGui::End();

		if (bSettingUI)
			ProjectSettingUI::RenderUI(setting, rootPath, bSettingUI);
	}

	SH_EDITOR_API void Project::CreateNewProject(const std::filesystem::path& dir)
	{
		if (!std::filesystem::create_directory(dir))
		{
			SH_INFO_FORMAT("Can't create directory: {}", dir.u8string());
		}
		
		CopyProjectTemplate(dir);

		OpenProject(dir);
	}

	SH_EDITOR_API void Project::OpenProject(const std::filesystem::path& dir)
	{
		rootPath = dir;
		assetPath = rootPath / "Assets";
		binaryPath = rootPath / "bin";
		libraryPath = rootPath / "Library";
		tempPath = rootPath / "temp";

		const auto settingPath = rootPath / "ProjectSetting.json";

		if (!std::filesystem::exists(assetPath) || !std::filesystem::exists(settingPath))
		{
			SH_ERROR_FORMAT("Wrong project directory: {}", dir.u8string());
			return;
		}
		isOpen = true;

		projectExplorer.SetRoot(assetPath);
		projectExplorer.Refresh();

		assetDatabase.SetProject(*this);
		assetDatabase.LoadDatabase(libraryPath / "AssetDB.json");
		assetDatabase.SetProjectDirectory(rootPath);
		assetDatabase.LoadAllAssets(assetPath, true);

		setting.Load(settingPath);
		
		SaveLatestProjectPath(dir);

		auto& gameManager = *game::GameManager::GetInstance();
		gameManager.LoadUserModule(binaryPath / "ShellEngineUser", true);
		LoadEditorPlugin(binaryPath / "ShellEngineUserEditor", true);

		ChangeSourcePath(dir); // CMakeLists.txt의 엔진 경로 바꾸는 함수

		if (!setting.lastWorldUUID.IsEmpty())
		{
			auto opt = assetDatabase.GetAssetOriginalPath(setting.lastWorldUUID);
			if (opt.has_value())
				LoadWorld(opt.value());
		}
		else
			NewWorld("New World");
	}

	SH_EDITOR_API void Project::NewWorld(const std::string& name)
	{
		auto& gameManager = *game::GameManager::GetInstance();

		editor::EditorWorld* newWorld = core::SObject::Create<editor::EditorWorld>(*this);

		gameManager.LoadWorld(newWorld->GetUUID());
	}

	SH_EDITOR_API void Project::SaveWorld()
	{
		for (auto& [uuid, worldPtr] : game::GameManager::GetInstance()->GetWorlds())
		{
			worldPtr->SaveWorldPoint(worldPtr->Serialize());
			assetDatabase.SetDirty(worldPtr);
		}
		assetDatabase.SaveAllAssets();
	}

	SH_EDITOR_API void Project::SaveAsWorld(const std::filesystem::path& worldAssetPath)
	{
		game::World* currentWorld = game::GameManager::GetInstance()->GetMainWorld();
		if (!core::IsValid(currentWorld))
			return;

		currentWorld->ReallocateUUIDS();

		std::ofstream os{ worldAssetPath };
		os << std::setw(4) << currentWorld->Serialize();
		os.close();
	}

	SH_EDITOR_API void Project::LoadWorld(const std::filesystem::path& worldAssetPath)
	{
		auto uuidOpt = assetDatabase.GetAssetUUID(worldAssetPath);
		if (!uuidOpt.has_value())
			return;

		core::SObject* obj = core::SObjectManager::GetInstance()->GetSObject(uuidOpt.value());
		if (obj == nullptr)
			return;

		game::World* world = core::reflection::Cast<game::World>(obj);
		if (world == nullptr)
			return;

		auto& gameManager = *game::GameManager::GetInstance();
		gameManager.LoadWorld(world->GetUUID());

		setting.lastWorldUUID = world->GetUUID();
	}
	SH_EDITOR_API auto Project::IsProjectOpen() const -> bool
	{
		return isOpen;
	}
	SH_EDITOR_API void Project::ReloadModule()
	{
		static bool bReloading = false;
		if (bReloading)
			return;
		bReloading = true;
		game::GameManager::GetInstance()->AddAterUpdateTask(
			[this]()
			{
				core::ModuleLoader loader{};
				loader.Clean(editorPlugin);
				LoadEditorPlugin(binaryPath / "ShellEngineUserEditor", true);
				bReloading = false;
			}
		);
		game::GameManager::GetInstance()->ReloadUserModule();

	}
	SH_EDITOR_API auto Project::GetLatestProjectPath() -> std::filesystem::path
	{
		return LoadLatestProjectPath();
	}
	SH_EDITOR_API auto Project::GetProjectSetting() const -> ProjectSetting&
	{
		return const_cast<ProjectSetting&>(setting);
	}
	SH_EDITOR_API void Project::OpenSettingUI()
	{
		bSettingUI = true;
	}
	SH_EDITOR_API void Project::Build()
	{
		BuildSystem builder{};
		if (setting.startingWorld.IsValid())
		{
			builder.Build(*this, binaryPath);
		}
		else
			SH_ERROR("Set the starting world first!");
	}

	void Project::RenderNameBar()
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, ImVec4{ 0.2, 0.2, 0.2, 1 });

		ImGui::BeginChild("Namebar", ImVec2{ 0, 0 });
		ImGui::SetCursorPosX(5.0f);
		ImGui::Text(projectExplorer.GetSelected().u8string().c_str());
		ImGui::EndChild();

		ImGui::PopStyleColor();
	}
	void Project::CopyProjectTemplate(const std::filesystem::path& targetDir)
	{
		std::filesystem::path projectTemplate{ exePath / "ProjectTemplate" };
		core::FileSystem::CopyAllFiles(projectTemplate, targetDir);

		ChangeSourcePath(targetDir);
	}
	void Project::LoadEditorPlugin(const std::filesystem::path& pluginPath, bool bCopy)
	{
		std::filesystem::path dllPath = pluginPath;
#if _WIN32
		if (pluginPath.has_extension())
		{
			if (pluginPath.extension() != ".dll")
				dllPath = pluginPath.parent_path() / std::filesystem::u8path(pluginPath.stem().u8string() + ".dll");
		}
		else
			dllPath = std::filesystem::u8path(pluginPath.u8string() + ".dll");

		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO_FORMAT("{} not found", dllPath.u8string());
			return;
		}
		if (bCopy)
		{
			auto pdbPath = pluginPath.parent_path() / std::filesystem::path(pluginPath.stem().u8string() + ".pdb");
			if (std::filesystem::exists(pdbPath))
				std::filesystem::remove(pdbPath);
			std::filesystem::path tempPath = pluginPath.parent_path() / "tempEditor.dll";
			std::filesystem::copy_file(dllPath, tempPath, std::filesystem::copy_options::overwrite_existing);

			dllPath = std::move(tempPath);
			originalPluginPath = pluginPath;
		}
#elif __linux__
		if (pluginPath.has_extension())
		{
			if (pluginPath.extension() != ".so")
				dllPath = pluginPath.parent_path() / std::filesystem::u8path("lib" + pluginPath.stem().u8string() + ".so");
		}
		else
			dllPath = std::filesystem::current_path() / pluginPath.parent_path() / std::filesystem::u8path("lib" + pluginPath.stem().u8string() + ".so");

		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO_FORMAT("{} not found", dllPath.u8string());
			return;
		}
		if (bCopy)
		{
			std::filesystem::path tempPath = pluginPath.parent_path() / "tempEditor.so";
			std::filesystem::copy_file(dllPath, tempPath, std::filesystem::copy_options::overwrite_existing);

			dllPath = std::move(tempPath);
			originalPluginPath = pluginPath;
		}
#endif

		core::ModuleLoader loader{};
		auto plugin = loader.Load(dllPath);
		if (!plugin.has_value())
			SH_ERROR_FORMAT("Failed to load module: {}", dllPath.u8string());
		else
		{
			editorPlugin = std::move(plugin.value());
		}
	}

	void Project::ChangeSourcePath(const std::filesystem::path& projectRootPath)
	{
		auto cmake = core::FileSystem::LoadText(projectRootPath / "CMakeLists.txt");
		if (cmake.has_value())
		{
			std::string cmakeStr = std::move(cmake.value());
			const std::string replacement = fmt::format("set(ENGINE_DIR {})", exePath.generic_u8string());
			cmakeStr = std::regex_replace(cmakeStr, engineDirRegex, replacement);
			core::FileSystem::SaveText(cmakeStr, projectRootPath / "CMakeLists.txt");
		}
	}

	void Project::SaveLatestProjectPath(const std::filesystem::path& path)
	{
		if (path.empty())
			return;

		core::FileSystem::SaveText(path.u8string(), "latestProjectPath");
	}

	auto Project::LoadLatestProjectPath() -> std::filesystem::path
	{
		auto opt = core::FileSystem::LoadText("latestProjectPath");
		if (opt.has_value())
			return std::filesystem::u8path(opt.value());
		else
			return {};
	}
}//namespace