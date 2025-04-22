#include "UI/Project.h"
#include "EditorWorld.h"
#include "EditorResource.h"
#include "AssetDatabase.h"
#include "AssetExtensions.h"
#include "BuildSystem.h"

#include "Render/Renderer.h"
#include "Render/Model.h"

#include "Core/FileSystem.h"

namespace sh::editor
{
	bool Project::bInitResource = false;

	Project::Project(EditorWorld& world) :
		world(world),
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
		std::string pathStr = path.u8string();
		std::string extension = path.extension().u8string();
		std::string payloadName = "asset";

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			AssetExtensions::Type extType = AssetExtensions::CheckType(extension);
			if (extType == AssetExtensions::Type::Model)
			{
				payloadName = core::reflection::GetType<render::Model>().name;
				item = world.models.GetResource(pathStr);
				if (item == nullptr)
					item = AssetDatabase::ImportAsset(world, path);
				ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(render::Model*));
			}
			else if (extType == AssetExtensions::Type::Material)
			{
				payloadName = core::reflection::GetTypeName<render::Material*>();
				item = world.materials.GetResource(pathStr);
				if (item == nullptr)
					item = AssetDatabase::ImportAsset(world, path);
				ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(render::Material*));
			}
			else if (extType == AssetExtensions::Type::Texture)
			{
				payloadName = core::reflection::GetTypeName<render::Texture>();
				item = world.textures.GetResource(pathStr);
				if (item == nullptr)
					item = AssetDatabase::ImportAsset(world, path);
				ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(render::Texture*));
			}
			ImGui::Text("%s", path.filename().u8string().c_str());
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
			AssetExtensions::Type extType = AssetExtensions::CheckType(extension);
			if (extType == AssetExtensions::Type::Model)
				icon = meshIcon;
			else if (extType == AssetExtensions::Type::Material)
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
		if (ImGui::ImageButton(path.u8string().c_str(), *icon, ImVec2{ iconSize, iconSize }))
		{
			selected = path;
			auto uuidStr = AssetDatabase::GetAssetUUID(path);
			if (uuidStr)
			{
				auto objPtr = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ uuidStr.value() });
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

	inline void Project::RenderNameBar()
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, ImVec4{ 0.2, 0.2, 0.2, 1 });

		ImGui::BeginChild("Namebar", ImVec2{0, 0});
		ImGui::SetCursorPosX(5.0f);
		ImGui::Text(selected.u8string().c_str());
		ImGui::EndChild();

		ImGui::PopStyleColor();
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
			return;
		}
		rootPath = dir;
		currentPath = dir;
		GetAllFiles(currentPath);
	}

	SH_EDITOR_API void Project::OpenProject(const std::filesystem::path& dir)
	{
		isOpen = true;

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
	SH_EDITOR_API auto Project::IsProjectOpen() const -> bool
	{
		return isOpen;
	}
	SH_EDITOR_API void Project::Build()
	{
		BuildSystem buildSystem{};
		buildSystem.Build(*this);
	}
}