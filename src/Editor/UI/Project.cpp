#include "UI/Project.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "AssetDatabase.h"
#include "AssetExtensions.h"

#include "Core/ModuleLoader.h"
#include "Core/FileSystem.h"
#include "Core/GarbageCollection.h"

#include "Render/Renderer.h"
#include "Render/Model.h"

#include "Game/ComponentModule.h"
#include "Game/GameObject.h"

namespace sh::editor
{
	bool Project::bInitResource = false;

	Project::Project(EditorWorld& world) :
		world(world),
		rootPath(std::filesystem::current_path()),
		currentPath(rootPath),
		assetDatabase(*AssetDatabase::GetInstance())
	{
		invisibleExtensions.push_back(".meta");

		InitResources();

		GetAllFiles(currentPath);
	}

	Project::~Project()
	{
		assetDatabase.SaveDatabase(libraryPath / "AssetDB.json");
	}

	void Project::InitResources()
	{
		if (bInitResource)
			return;
		bInitResource = true;

		auto resource = EditorResource::GetInstance();
		folderIcon = resource->GetIcon(EditorResource::Icon::Folder);
		fileIcon = resource->GetIcon(EditorResource::Icon::File);
		meshIcon = resource->GetIcon(EditorResource::Icon::Mesh);
	}

	void Project::GetAllFiles(const std::filesystem::path& path)
	{
		foldersPath.clear();
		filesPath.clear();
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (std::filesystem::is_directory(entry.path()))
				foldersPath.push_back(entry.path());
			else if (std::filesystem::is_regular_file(entry.path()))
				filesPath.push_back(entry.path());
		}
		std::sort(foldersPath.begin(), foldersPath.end());
		std::sort(filesPath.begin(), filesPath.end());
	}
	auto Project::GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string
	{
		std::string result = path.filename().u8string();
		float currentSize = ImGui::CalcTextSize(result.c_str()).x;
		while (currentSize > maxSize)
		{
			result.pop_back();
			currentSize = ImGui::CalcTextSize(result.c_str()).x;
		}
		return result;
	}
	void Project::RenderParentFolder()
	{
		ImGui::BeginGroup();
		ImGui::ImageButton("../", *folderIcon, ImVec2{ iconSize, iconSize }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, iconBackgroundColor);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			currentPath = currentPath.parent_path();
			GetAllFiles(currentPath);
		}
		float textWidth = ImGui::CalcTextSize("../").x;
		float textOffset = (iconSize - textWidth) / 2.f;
		if (textOffset >= 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::Text("../");
		ImGui::EndGroup();
	}

	void Project::SetDragItem(const std::filesystem::path& path)
	{
		void* item = nullptr;
		std::string pathStr = path.u8string();
		std::string extension = path.extension().u8string();
		std::string payloadName = "asset";

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			auto uuidOpt = assetDatabase.GetAssetUUID(path);
			if (!uuidOpt.has_value())
				item = assetDatabase.ImportAsset(world, path);
			else
				item = core::SObjectManager::GetInstance()->GetSObject(uuidOpt.value());

			assert(item != nullptr);
			if (item == nullptr)
				return;

			payloadName = reinterpret_cast<core::SObject*>(item)->GetType().type.name;
			ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(void*));

			ImGui::Text("%s", path.filename().u8string().c_str());
			ImGui::EndDragDropSource();
		}

	}

	auto Project::GetIcon(const std::filesystem::path& path) const -> const game::GUITexture*
	{
		std::string extension = path.extension().string();
		const game::GUITexture* icon = folderIcon;
		if (std::filesystem::is_regular_file(path))
		{
			icon = fileIcon;
			AssetExtensions::Type extType = AssetExtensions::CheckType(extension);
			if (extType == AssetExtensions::Type::Model)
				icon = meshIcon;
			else if (extType == AssetExtensions::Type::Material)
				icon = EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Material);
		}
		return icon;
	}

	bool Project::RenderFile(const std::filesystem::path& path, float& cursorX, float spacing, float width)
	{
		cursorX = ImGui::GetCursorPosX();
		if (cursorX + iconSize > width)
		{
			ImGui::NewLine();
			cursorX = ImGui::GetCursorPosX();
		}
		ImGui::BeginGroup();
		const game::GUITexture* icon = GetIcon(path);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, iconBackgroundColor);
		if (ImGui::ImageButton(path.u8string().c_str(), *icon, ImVec2{ iconSize, iconSize }))
		{
			selected = path;
			auto uuidStr = assetDatabase.GetAssetUUID(path);
			if (uuidStr)
			{
				auto objPtr = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ uuidStr.value() });
				if (core::IsValid(objPtr))
				{
					world.ClearSelectedObjects();
					world.AddSelectedObject(objPtr);
				}
			}
		}
		SetDragItem(path);

		ImGui::PopStyleColor();
		// 폴더면 더블 클릭 시 경로 변경
		if (std::filesystem::is_directory(path))
		{
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				ImGui::EndGroup();
				currentPath = path;
				GetAllFiles(currentPath);
				return false;
			}
		}
		std::string name = GetElideFileName(path, iconSize);
		float textWidth = ImGui::CalcTextSize(name.c_str()).x;
		float textOffset = (iconSize - textWidth) / 2.f;
		if (textOffset >= 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::Text("%s", name.c_str());
		ImGui::EndGroup();

		ImGui::SameLine(0.0f, spacing);
		cursorX += iconSize + spacing;

		return true;
	}

	void Project::ShowRightClickPopup()
	{
		if (ImGui::BeginPopupContextWindow("ProjectRightClickPopup"))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder"))
				{
					core::FileSystem::CreateFolder(currentPath, "NewFolder");

					GetAllFiles(currentPath);
				}
				if (ImGui::MenuItem("Material"))
				{
					static render::Shader* defaultShader = world.shaders.GetResource("DefaultShader");
					assert(defaultShader);
					std::string name{ core::FileSystem::CreateUniqueFileName(currentPath, "NewMaterial.mat") };
					auto mat = world.materials.AddResource(name, render::Material{ defaultShader });
					mat->SetName(name);
					mat->Build(*world.renderer.GetContext());
					assetDatabase.CreateAsset(world, currentPath / name, *mat);

					GetAllFiles(currentPath);
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	void Project::RenderNameBar()
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, ImVec4{ 0.2, 0.2, 0.2, 1 });

		ImGui::BeginChild("Namebar", ImVec2{0, 0});
		ImGui::SetCursorPosX(5.0f);
		ImGui::Text(selected.u8string().c_str());
		ImGui::EndChild();

		ImGui::PopStyleColor();
	}

	void Project::LoadUserModule()
	{
		game::ComponentModule* componentModule = game::ComponentModule::GetInstance();

		for (const auto& componentInfo : userComponents)
		{
			componentModule->DestroyComponent(componentInfo.first);
		}
		userComponents.clear();
#if _WIN32
		auto dllPath = binaryPath / "ShellEngineUser.dll";
		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO("ShellEngineUser.dll not found");
			return;
		}
		auto pluginPath = binaryPath / "temp.dll";
		std::filesystem::copy_file(dllPath, pluginPath, std::filesystem::copy_options::overwrite_existing);
#elif __linux__
		auto dllPath = binaryPath / "libShellEngineUser.so";
		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO("libShellEngineUser.so not found");
			return;
		}
		auto pluginPath = binaryPath / "temp.so";
		std::filesystem::copy_file(dllPath, pluginPath, std::filesystem::copy_options::overwrite_existing);
#endif
		core::ModuleLoader loader{};
		auto plugin = loader.Load(pluginPath);
		assert(plugin.has_value());
		if (!plugin.has_value())
			SH_ERROR_FORMAT("Can't load module: {}", pluginPath.u8string());
		else
		{
			userPlugin = std::move(plugin.value());

			for (const auto& componentInfo : componentModule->GetWaitingComponents())
				userComponents.push_back({ componentInfo.name, &componentInfo.type });

			componentModule->RegisterWaitingComponents();
		}
	}

	void Project::CopyProjectTemplate(const std::filesystem::path& targetDir)
	{
		std::filesystem::path projectTemplate{ std::filesystem::current_path() / "ProjectTemplate" };
		core::FileSystem::CopyAllFiles(projectTemplate, targetDir);

		auto cmake = core::FileSystem::LoadText(targetDir / "CMakeLists.txt");
		if (cmake.has_value())
		{
			std::string cmakeStr = std::move(cmake.value());
			std::string directoryStr = "\"Here is directory\"";
			auto it = cmakeStr.find(directoryStr);
			cmakeStr = cmakeStr.replace(it, directoryStr.length(), std::filesystem::current_path().u8string());
			core::FileSystem::SaveText(cmakeStr, targetDir / "CMakeLists.txt");
		}
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

		float availableWidth = ImGui::GetContentRegionAvail().x;
		float cursorX = ImGui::GetCursorPosX();
		float spacing = ImGui::GetStyle().ItemSpacing.x;

		if (currentPath != rootPath)
		{
			RenderParentFolder();
			ImGui::SameLine();
		}
		for (auto& path : foldersPath)
		{
			if (!RenderFile(path, cursorX, spacing, availableWidth))
				break;
		}
		for (auto& path : filesPath)
		{
			if (std::find(invisibleExtensions.begin(), invisibleExtensions.end(), path.extension().string()) != invisibleExtensions.end())
				continue;
			if (!RenderFile(path, cursorX, spacing, availableWidth))
				break;
		}

		ShowRightClickPopup();

		ImGui::EndChild();
		ImGui::PopStyleVar();
		RenderNameBar();
		ImGui::End();
	}

	SH_EDITOR_API void Project::CreateNewProject(const std::filesystem::path& dir)
	{
		if (!std::filesystem::create_directory(dir))
		{
			SH_INFO_FORMAT("Can't create directory: {}", dir.u8string());
		}
		
		CopyProjectTemplate(dir);

		rootPath = dir;
		currentPath = dir;
		GetAllFiles(currentPath);
	}

	SH_EDITOR_API void Project::OpenProject(const std::filesystem::path& dir)
	{
		isOpen = true;

		rootPath = dir;
		assetPath = rootPath / "Assets";
		binaryPath = rootPath / "bin";
		libraryPath = rootPath / "Library";
		currentPath = dir;
		GetAllFiles(currentPath);

		LoadUserModule();

		assetDatabase.LoadDatabase(libraryPath / "AssetDB.json");
		assetDatabase.SetProjectDirectory(rootPath);
		assetDatabase.LoadAllAssets(world, assetPath, true);
	}

	SH_EDITOR_API void Project::SaveWorld(const std::string& name)
	{
		std::ofstream os{ assetPath / (name + ".world") };
		os << std::setw(4) << world.Serialize();
		os.close();
	}
	SH_EDITOR_API void Project::LoadWorld(const std::string& name)
	{
		world.AddBeforeSyncTask(
			[&, name]()
			{
				auto file = core::FileSystem::LoadText(assetPath / (name + ".world"));
				if (file)
				{
					world.Deserialize(core::Json::parse(file.value()));
				}
			}
		);
	}
	SH_EDITOR_API auto Project::IsProjectOpen() const -> bool
	{
		return isOpen;
	}
	SH_EDITOR_API void Project::ReloadModule()
	{
		world.AddBeforeSyncTask(
			[&]()
			{
				bool bSave = false;
				if (userPlugin.handle != nullptr)
				{
					SaveWorld("temp");

					for (auto obj : world.GetGameObjects())
					{
						for (auto component : obj->GetComponents())
						{
							if (component == nullptr)
								continue;
							for (auto& userComponent : userComponents)
							{
								if (component->GetType() == *userComponent.second)
								{
									component->Destroy();
								}
							}
						}
					}
					core::GarbageCollection::GetInstance()->Collect();

					core::ModuleLoader loader{};
					loader.Clean(userPlugin);

					bSave = true;
				}

				LoadUserModule();

				if (bSave)
				{
					LoadWorld("temp");
					core::GarbageCollection::GetInstance()->Collect();
				}
			}
		);
	}
	SH_EDITOR_API auto Project::GetAssetPath() const -> const std::filesystem::path&
	{
		return assetPath;
	}
	SH_EDITOR_API auto Project::GetBinPath() const -> const std::filesystem::path&
	{
		return binaryPath;
	}
}