#include "Project.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "ModelLoader.h"
#include "AssetDatabase.h"

#include "Render/Renderer.h"

#include "Core/FileSystem.h"

namespace sh::editor
{
	bool Project::bInitResource = false;

	Project::Project(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui), world(world),
		rootPath(std::filesystem::current_path()),
		currentPath(rootPath)
	{
		invisibleExtensions.push_back(".meta");

		InitResources();

		GetAllFiles(currentPath);
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
	inline void Project::RenderParentFolder()
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

	inline void Project::SetDragItem(const std::filesystem::path& path)
	{
		void* item = nullptr;
		std::string pathStr = path.string();
		std::string extension = path.extension().string();
		std::string payloadName = "asset";

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			if (extension == ".obj")
			{
				payloadName = core::reflection::GetTypeName<render::Mesh*>();
				item = world.meshes.GetResource(pathStr);
				if (item == nullptr)
				{
					ModelLoader loader{ *world.renderer.GetContext()};
					item = world.meshes.AddResource(pathStr, loader.Load(pathStr));
				}
			}
			else if (extension == ".mat")
			{
				payloadName = core::reflection::GetTypeName<render::Material*>();
				item = world.materials.GetResource(pathStr);
				if (item == nullptr)
					AssetDatabase::ImportAsset(world, path);
			}
			else if (extension == ".png" || extension == ".jpg")
			{
				payloadName = core::reflection::GetTypeName<render::Texture*>();
				item = world.textures.GetResource(pathStr);
				if (item == nullptr)
					AssetDatabase::ImportAsset(world, path);
			}
			ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(render::Mesh*));
			ImGui::Text("%s", path.filename().c_str());
			ImGui::EndDragDropSource();
		}

	}

	inline auto Project::GetIcon(const std::filesystem::path& path) const -> const game::GUITexture*
	{
		std::string extension = path.extension().string();
		const game::GUITexture* icon = folderIcon;
		if (std::filesystem::is_regular_file(path))
		{
			icon = fileIcon;
			if (extension == ".obj")
				icon = meshIcon;
			else if (extension == ".mat")
				icon = EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Material);
		}
		return icon;
	}

	inline bool Project::RenderFile(const std::filesystem::path& path, float& cursorX, float spacing, float width)
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
		if (ImGui::ImageButton(path.string().c_str(), *icon, ImVec2{ iconSize, iconSize }))
		{
			auto uuidStr = AssetDatabase::GetAssetUUID(path);
			if (uuidStr)
			{
				auto objPtr = core::SObjectManager::GetInstance()->GetSObject(uuidStr.value().ToString());
				if (core::IsValid(objPtr))
					world.SetSelectedObject(objPtr);
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

	inline void Project::ShowRightClickPopup()
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
					AssetDatabase::CreateAsset(world, currentPath / name, *mat);

					GetAllFiles(currentPath);
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	SH_EDITOR_API void Project::Update()
	{
	}
	SH_EDITOR_API void Project::Render()
	{
		static ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Project", nullptr, style);

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
		
		ImGui::End();
	}

	SH_EDITOR_API void Project::CreateNewProject(const std::filesystem::path& dir)
	{
		if (!std::filesystem::create_directory(dir))
		{
			SH_INFO_FORMAT("Can't create directory: {}", dir.u8string());
			return;
		}
		rootPath = dir;
		currentPath = dir;
		GetAllFiles(currentPath);
	}

	SH_EDITOR_API void Project::OpenProject(const std::filesystem::path& dir)
	{
		rootPath = dir;
		assetPath = rootPath / "Assets";
		currentPath = dir;
		GetAllFiles(currentPath);

		AssetDatabase::LoadAllAssets(world, dir, true);
	}

	SH_EDITOR_API void Project::SaveWorld()
	{
		std::ofstream os{ assetPath / "test.world" };
		os << std::setw(4) << world.Serialize();
		os.close();
	}
	SH_EDITOR_API void Project::LoadWorld()
	{
		auto file = core::FileSystem::LoadText(assetPath / "test.world");
		if (file)
		{
			world.Deserialize(core::Json::parse(file.value()));
		}
	}
}