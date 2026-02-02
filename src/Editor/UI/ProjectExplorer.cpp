#include "UI/ProjectExplorer.h"
#include "UI/Project.h"
#include "EditorResource.h"
#include "AssetExtensions.h"
#include "AssetDatabase.h"
#include "EditorWorld.h"

#include "Core/FileSystem.h"

#include "Game/GUITexture.h"
#include "Game/Prefab.h"
#include "Game/GameManager.h"

namespace sh::editor
{
	ProjectExplorer::ProjectExplorer()
	{
		invisibleExtensions.push_back(".meta");
	}
	SH_EDITOR_API void ProjectExplorer::SetRoot(const std::filesystem::path& root)
	{
		rootPath = root;
		currentPath = root;
		Refresh();
	}
	SH_EDITOR_API void ProjectExplorer::Refresh()
	{
		if (!std::filesystem::exists(currentPath))
			return;
		folders.clear();
		files.clear();
		for (const auto& e : std::filesystem::directory_iterator(currentPath))
		{
			if (std::filesystem::is_directory(e.path()))
				folders.push_back({ e.path(), GetIcon(e.path()), true });
			else if (std::filesystem::is_regular_file(e.path())) 
				files.push_back({ e.path(), GetIcon(e.path()), false });
		}
		sortList(folders);
		sortList(files);
	}
	SH_EDITOR_API void ProjectExplorer::Render(float availableWidth)
	{
		float cursorX = ImGui::GetCursorPosX();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;

		if (currentPath != rootPath)
		{
			RenderParent();
			ImGui::SameLine();
		}
		for (auto& folder : folders)
		{
			if (!RenderItem(folder, cursorX, spacing, availableWidth)) 
				break;
		}
		for (auto& file : files)
		{
			if (IsInvisibleExtension(file.path.extension().string()))
				continue;
			if (!RenderItem(file, cursorX, spacing, availableWidth))
				break;
		}
		RenderRightClickPopup();

		if (bChangeFolderState && ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
			bChangeFolderState = false;
	}
	SH_EDITOR_API auto ProjectExplorer::GetCurrentPath() const -> const std::filesystem::path&
	{
		return currentPath;
	}
	SH_EDITOR_API void ProjectExplorer::SetCurrentPath(const std::filesystem::path& p)
	{
		currentPath = p; 
		Refresh();
	}
	SH_EDITOR_API void ProjectExplorer::SetSelected(const std::filesystem::path& path)
	{
		selected = path;
	}
	SH_EDITOR_API void ProjectExplorer::ResetSelected()
	{
		selected.clear();
	}
	SH_EDITOR_API auto ProjectExplorer::GetSelected() const -> const std::filesystem::path&
	{
		return selected;
	}
	auto ProjectExplorer::GetIcon(const std::filesystem::path& path) const -> const game::GUITexture*
	{
		if (std::filesystem::is_directory(path))
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Folder);
		// file
		auto extType = AssetExtensions::CheckType(path.extension().string());
		switch (extType)
		{
		case AssetExtensions::Type::Model:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Mesh);
		case AssetExtensions::Type::Material:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Material);
		case AssetExtensions::Type::Texture:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Image);
		case AssetExtensions::Type::Shader:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Shader);
		case AssetExtensions::Type::World:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::World);
		case AssetExtensions::Type::Prefab:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Prefab);
		default:
			return EditorResource::GetInstance()->GetIcon(EditorResource::Icon::File);
		}
	}
	auto ProjectExplorer::GetElideFileName(const std::filesystem::path& path, float maxSize) const -> std::string
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
	auto ProjectExplorer::IsInvisibleExtension(const std::string& ext) const -> bool
	{
		return std::find(invisibleExtensions.begin(), invisibleExtensions.end(), ext) != invisibleExtensions.end();
	}
	auto ProjectExplorer::RenderItem(const FileItem& fi, float& cursorX, float spacing, float width) -> bool
	{
		cursorX = ImGui::GetCursorPosX();
		if (cursorX + iconSize > width)
		{
			ImGui::NewLine();
			cursorX = ImGui::GetCursorPosX();
		}

		ImGui::BeginGroup();
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, iconBackgroundColor);
		ImGui::ImageButton(fi.path.u8string().c_str(), *fi.icon, ImVec2{ iconSize, iconSize });

		if (!bChangeFolderState && ImGui::IsItemHovered() && (ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Right)))
		{
			OnItemClicked(fi.path);
		}

		ImGui::PopStyleColor();

		SetItemDragTarget(fi.path);

		if (fi.isDirectory)
		{
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				ImGui::EndGroup();
				SetCurrentPath(fi.path);
				bChangeFolderState = true;
				return false;
			}
			SetFolderDragTarget(fi.path);
		}

		std::string name = GetElideFileName(fi.path, iconSize);
		float textWidth = ImGui::CalcTextSize(name.c_str()).x;
		float textOffset = (iconSize - textWidth) / 2.f;
		if (textOffset >= 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::Text("%s", name.c_str());
		ImGui::EndGroup();

		ImGui::SameLine(0.0f, spacing);
		cursorX += iconSize + spacing;
		return true;
	}
	void  ProjectExplorer::RenderParent()
	{
		ImGui::BeginGroup();
		ImGui::ImageButton("../", *EditorResource::GetInstance()->GetIcon(EditorResource::Icon::Folder), ImVec2{ iconSize, iconSize }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, iconBackgroundColor);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			ImGui::EndGroup();
			SetCurrentPath(currentPath.parent_path());
			bChangeFolderState = true;
			return;
		}
		SetFolderDragTarget(currentPath.parent_path());
		float textWidth = ImGui::CalcTextSize("../").x;
		float textOffset = (iconSize - textWidth) / 2.f;
		if (textOffset >= 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::Text("../");
		ImGui::EndGroup();
	}
	void ProjectExplorer::RenderRightClickPopup()
	{
		if (ImGui::BeginPopupContextWindow("ProjectRightClickPopup"))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder"))
				{
					core::FileSystem::CreateFolder(currentPath, "NewFolder");
					Refresh();
				}
				if (ImGui::MenuItem("Material"))
				{
					static render::Shader* defaultShader = EditorResource::GetInstance()->GetShader("ErrorShader");
					assert(defaultShader);
					std::string name{ core::FileSystem::CreateUniqueFileName(currentPath, "NewMaterial.mat") };

					auto mat = core::SObject::Create<render::Material>(defaultShader);
					mat->SetName(name);
					mat->Build(*game::GameManager::GetInstance()->GetRenderer().GetContext());
					AssetDatabase::GetInstance()->CreateAsset(currentPath / name, *mat);
					Refresh();
				}
				ImGui::EndMenu();
			}
			if (!selected.empty())
			{
				if (ImGui::BeginMenu("Rename"))
				{
					std::string name = selected.filename().u8string();
					const std::filesystem::path parent = selected.parent_path();
					if (ImGui::InputText("##Rename", &name, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (std::filesystem::is_directory(selected))
						{
							// TODO...
							// 하위 파일들을 전부 찾아내고 assetdatabase를 갱신해야함
						}
						else
						{
							auto uuidOpt = AssetDatabase::GetInstance()->GetAssetUUID(selected);
							if (uuidOpt.has_value())
							{
								const std::filesystem::path metaPath = parent / std::filesystem::u8path(selected.filename().u8string() + ".meta");
								const std::filesystem::path newPath = parent / std::filesystem::u8path(name);
								std::filesystem::rename(selected, newPath);
								if (std::filesystem::exists(metaPath))
									std::filesystem::rename(metaPath, parent / std::filesystem::u8path(name + ".meta"));

								AssetDatabase::GetInstance()->AssetWasMoved(uuidOpt.value(), newPath);

								selected = newPath;

								Refresh();
							}
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Delete"))
				{
					auto uuidOpt = AssetDatabase::GetInstance()->GetAssetUUID(selected);
					if (uuidOpt.has_value())
					{
						AssetDatabase::GetInstance()->DeleteAsset(uuidOpt.value());

						selected.clear();

						Refresh();
					}
					else
					{
						std::filesystem::remove(selected);
						selected.clear();
						Refresh();
					}
				}
			}
			if (ImGui::MenuItem("Refresh"))
			{
				Refresh();
			}
			ImGui::EndPopup();
		}
	}
	void ProjectExplorer::SetItemDragTarget(const std::filesystem::path& path)
	{
		void* item = nullptr;
		std::string pathStr = path.u8string();
		std::string extension = path.extension().u8string();
		std::string payloadName = "asset";

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			auto uuidOpt = AssetDatabase::GetInstance()->GetAssetUUID(path);
			if (!uuidOpt.has_value())
				item = AssetDatabase::GetInstance()->ImportAsset(path);
			else
				item = core::SObjectManager::GetInstance()->GetSObject(uuidOpt.value());

			if (item == nullptr)
			{
				ImGui::EndDragDropSource();
				return;
			}

			payloadName = reinterpret_cast<core::SObject*>(item)->GetType().type.name;
			ImGui::SetDragDropPayload(payloadName.c_str(), &item, sizeof(void*));

			ImGui::Text("%s", path.filename().u8string().c_str());
			ImGui::EndDragDropSource();
		}
	}
	void ProjectExplorer::SetFolderDragTarget(const std::filesystem::path& folderPath)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = nullptr;
			const std::string_view assetTypes[] =
			{
				core::reflection::GetType<render::Texture>().name,
				core::reflection::GetType<render::Material>().name,
				core::reflection::GetType<render::Model>().name,
				core::reflection::GetType<render::Shader>().name,
				core::reflection::GetType<EditorWorld>().name,
				core::reflection::GetType<game::Prefab>().name,
				core::reflection::GetType<game::World>().name,
			};
			for (const std::string_view& assetType : assetTypes)
			{
				payload = ImGui::AcceptDragDropPayload(std::string{ assetType }.c_str());
				if (payload != nullptr && payload->Data != nullptr)
				{
					core::SObject* objPtr = *reinterpret_cast<core::SObject**>(payload->Data);
					AssetDatabase::GetInstance()->MoveAssetToDirectory(objPtr->GetUUID(), folderPath);
					Refresh();
					break;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
	void ProjectExplorer::OnItemClicked(const std::filesystem::path& path)
	{
		selected = path;
		auto uuidStrOpt = AssetDatabase::GetInstance()->GetAssetUUID(path);
		if (uuidStrOpt)
		{
			auto objPtr = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ uuidStrOpt.value() });
			if (core::IsValid(objPtr))
			{
				auto& gameManager = *game::GameManager::GetInstance();
				for (auto& [uuid, worldPtr] : gameManager.GetWorlds())
				{
					auto& world = static_cast<EditorWorld&>(*worldPtr);
					world.ClearSelectedObjects();
					world.AddSelectedObject(objPtr);
				}
			}
		}
	}
}//namespace