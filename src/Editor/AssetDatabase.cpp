#include "AssetDatabase.h"
#include "AssetExtensions.h"
#include "Meta.h"
#include "UI/Project.h"
#include "EditorWorld.h"

#include "Core/FileSystem.h"
#include "Core/Asset.h"
#include "Core/AssetImporter.h"
#include "Core/AssetExporter.h"

#include "Render/Renderer.h"
#include "Render/Model.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/VulkanImpl/VulkanContext.h"

#include "Game/GameManager.h"
#include "Game/World.h"
#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"
#include "Game/MeshLoader.h"
#include "Game/MaterialLoader.h"
#include "Game/ShaderLoader.h"
#include "Game/WorldLoader.h"
#include "Game/AssetLoaderFactory.h"
#include "Game/TextureAsset.h"
#include "Game/ModelAsset.h"
#include "Game/MeshAsset.h"
#include "Game/MaterialAsset.h"
#include "Game/ShaderAsset.h"
#include "Game/WorldAsset.h"

#include <random>
#include <istream>
#include <ostream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <chrono>
namespace sh::editor
{
	auto AssetDatabase::GetMetaDirectory(const std::filesystem::path& assetPath) -> std::filesystem::path
	{
		std::filesystem::path metaPath{ assetPath.parent_path() / assetPath.filename() };
		metaPath += ".meta";
		return metaPath;
	}
	auto AssetDatabase::HasMetaFile(const std::filesystem::path& dir) -> std::optional<std::filesystem::path>
	{
		if (!std::filesystem::exists(dir))
			return std::nullopt;
		std::filesystem::path metaFileDir = GetMetaDirectory(dir);
		if (!std::filesystem::exists(metaFileDir))
			return std::nullopt;
		return metaFileDir;
	}
	void AssetDatabase::SaveMaterial(render::Material* mat, const std::filesystem::path& dir)
	{
		if (!core::IsValid(mat))
			return;

		core::Json matJson{ mat->Serialize() };

		std::ofstream os{ dir };
		os << std::setw(4) << matJson;
		os.close();

		Meta meta{};
		meta.Save(*mat, GetMetaDirectory(dir), false);
	}

	SH_EDITOR_API auto AssetDatabase::ImportAsset(const std::filesystem::path& dir) -> core::SObject*
	{
		if (!std::filesystem::exists(dir))
			return nullptr;
		if (std::filesystem::is_directory(dir))
			return nullptr;
		
		const std::string extension = dir.extension().string();

		auto type = AssetExtensions::CheckType(extension);
		if (type == AssetExtensions::Type::Texture)
		{
			static game::TextureLoader loader{ *project->renderer.GetContext() };
			render::Texture* texPtr = static_cast<render::Texture*>(LoadAsset(dir, loader, true));
			if (texPtr != nullptr)
				project->loadedAssets.AddResource(texPtr->GetUUID(), texPtr);
			return texPtr;
		}
		if (type == AssetExtensions::Type::Model)
		{
			static game::ModelLoader loader{ *project->renderer.GetContext() };

			const bool isAssetChanged = IsAssetChanged(dir);
			render::Model* modelPtr = static_cast<render::Model*>(LoadAsset(dir, loader, true));
			if (modelPtr != nullptr)
			{
				project->loadedAssets.AddResource(modelPtr->GetUUID(), modelPtr);

				if (isAssetChanged)
				{
					for (const auto& mesh : modelPtr->GetMeshes())
					{
						if (mesh == nullptr)
							continue;
						const std::filesystem::path cachePath{ libPath / fmt::format("{}.asset", mesh->GetUUID().ToString()) };
						ExportAsset(*mesh, cachePath);
						paths.insert_or_assign(mesh->GetUUID(), AssetInfo{ std::filesystem::path{}, std::filesystem::relative(cachePath, projectPath) });
					}
				}
			}
			return modelPtr;
		}
		if (type == AssetExtensions::Type::Shader)
		{
			assert(project->renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan);
			if (project->renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan)
			{
				static render::vk::VulkanShaderPassBuilder passBuilder{ static_cast<render::vk::VulkanContext&>(*project->renderer.GetContext()) };
				static game::ShaderLoader loader{ &passBuilder };
				loader.SetCachePath(projectPath / "temp");
				render::Shader* shaderPtr = static_cast<render::Shader*>(LoadAsset(dir, loader, false));
				if (shaderPtr != nullptr)
					project->loadedAssets.AddResource(shaderPtr->GetUUID(), shaderPtr);
				return shaderPtr;
			}
		}
		if (type == AssetExtensions::Type::Material)
		{
			static game::MaterialLoader loader{ *project->renderer.GetContext() };
			render::Material* matPtr = static_cast<render::Material*>(LoadAsset(dir, loader, false));
			if (matPtr != nullptr)
				project->loadedAssets.AddResource(matPtr->GetUUID(), matPtr);
			return matPtr;
		}
		if (type == AssetExtensions::Type::World)
		{
			static game::WorldLoader loader{ project->renderer, project->gui };
			game::World* worldPtr = static_cast<game::World*>(LoadAsset(dir, loader, false));
			if (worldPtr != nullptr)
				project->loadedAssets.AddResource(worldPtr->GetUUID(), worldPtr);
			return worldPtr;
		}
		return nullptr;
	}

	SH_EDITOR_API void AssetDatabase::SaveAllAssets()
	{
		for (auto obj : dirtyObjs)
		{
			if (!core::IsValid(obj))
				continue;

			auto it = paths.find(obj->GetUUID());
			if (it == paths.end())
				continue;
			const auto& [originalPath, cachePath] = it->second;

			if (!std::filesystem::exists(projectPath / originalPath))
			{
				SH_ERROR_FORMAT("{} is not exists!", originalPath.u8string());
				continue;
			}
			uint64_t writeTime = std::filesystem::last_write_time(projectPath / originalPath).time_since_epoch().count();

			if (obj->GetType() == render::Material::GetStaticType())
			{
				SaveMaterial(static_cast<render::Material*>(obj), projectPath / originalPath);
			}
			else if (obj->GetType() == render::Texture::GetStaticType())
			{
				render::Texture* texture = reinterpret_cast<render::Texture*>(obj);
				Meta meta{};
				meta.SaveWithObj(*texture, GetMetaDirectory(projectPath / originalPath), false);
			}
			else if (obj->GetType().IsChildOf(game::World::GetStaticType()))
			{
				const game::World* world = static_cast<game::World*>(obj);

				const auto worldPath = projectPath / originalPath;

				std::ofstream os{ worldPath };
				os << std::setw(4) << world->Serialize();
				os.close();

				Meta meta{};
				meta.Save(*world, GetMetaDirectory(projectPath / originalPath), false);
			}
		}
		dirtyObjs.clear();
	}

	void AssetDatabase::LoadAllAssetsHelper(const std::filesystem::path& dir, bool recursive)
	{
		for (auto& entry : std::filesystem::directory_iterator(dir))
		{
			if (entry.is_directory())
			{
				if (recursive)
					LoadAllAssetsHelper(entry.path(), recursive);
			}
			else if (entry.is_regular_file())
			{
				if (entry.path().extension() == ".meta")
					continue;

				int priority = 0;
				if (entry.path().extension() == ".mat")
					priority = -1;
				loadingAssetsQueue.push(AssetLoadData{ priority, entry.path() });
			}
		}
	}
	auto AssetDatabase::LoadAsset(const std::filesystem::path& path, core::IAssetLoader& loader, bool bMetaSaveWithObj) -> core::SObject*
	{
		std::filesystem::path metaDir{ GetMetaDirectory(path) };
		std::filesystem::path relativePath{ std::filesystem::relative(path, projectPath) };
		const int64_t writeTime = std::filesystem::last_write_time(path).time_since_epoch().count();

		Meta meta{};
		if (meta.Load(metaDir))
		{
			auto it = paths.find(meta.GetUUID());
			if (it != paths.end())
			{
				const auto& [filePath, assetPath] = it->second;
				std::unique_ptr<core::Asset> asset = core::AssetImporter::Load(projectPath / assetPath);
				if (asset != nullptr)
				{
					int64_t assetWriteTime = asset->GetWriteTime();
					bool bMetaChanged = meta.IsChanged();
					if (assetWriteTime == writeTime && !bMetaChanged) // 다른 경우 에셋이 변경됐다는 뜻
					{
						core::SObject* objPtr = loader.Load(*asset.get());
						if (objPtr == nullptr)
							return nullptr;

						objPtr->SetName(path.stem().u8string());

						uuids.insert_or_assign(relativePath, objPtr->GetUUID());
						return objPtr;
					}
					else
					{
						SH_INFO_FORMAT("Asset {} was changed!", relativePath.u8string());
					}
				}
			}
		}
		core::SObject* objPtr = loader.Load(path);
		if (objPtr == nullptr)
			return nullptr;

		if (meta.IsLoad())
			meta.DeserializeSObject(*objPtr);
		else
		{
			objPtr->SetName(path.stem().u8string());
		}

		std::filesystem::path cachePath{ libPath / fmt::format("{}.asset", objPtr->GetUUID().ToString()) };
		if (!ExportAsset(*objPtr, cachePath, writeTime))
			return nullptr;

		uuids.insert_or_assign(relativePath, objPtr->GetUUID());
		paths.insert_or_assign(objPtr->GetUUID(), AssetInfo{ relativePath, std::filesystem::relative(cachePath, projectPath) });

		if (bMetaSaveWithObj)
			meta.SaveWithObj(*objPtr, metaDir);
		else
			meta.Save(*objPtr, metaDir);

		return objPtr;
	}
	AssetDatabase::AssetDatabase()
	{
		projectPath = std::filesystem::current_path();

		onDestroyListener.SetCallback(
			[&](const core::SObject* destoryedObj)
			{
				auto it = std::find(dirtyObjs.begin(), dirtyObjs.end(), destoryedObj);
				if (it != dirtyObjs.end())
					dirtyObjs.erase(it);
			}
		);
	}
	SH_EDITOR_API void AssetDatabase::SetProject(Project& project)
	{
		this->project = &project;
	}
	SH_EDITOR_API void AssetDatabase::SaveDatabase(const std::filesystem::path& dir)
	{
		core::Json json{};
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			const core::UUID& uuid = it->first;
			const auto& originalPath = it->second.originalPath;
			const auto& cachePath = it->second.cachePath;

			core::Json uuidJson{};
			uuidJson["0"] = originalPath.u8string();
			uuidJson["1"] = cachePath.u8string();
			json[uuid.ToString()] = uuidJson;
		}
		
		std::ofstream os{ dir };
		os << std::setw(4) << json;
		os.close();
	}
	SH_EDITOR_API auto AssetDatabase::LoadDatabase(const std::filesystem::path& dir) -> bool
	{
		auto opt = core::FileSystem::LoadText(dir);
		if (!opt.has_value())
			return false;

		paths.clear();

		core::Json json{ core::Json::parse(opt.value()) };
		if (json.empty())
		{
			SH_ERROR_FORMAT("Parsing failed: {}", dir.u8string());
		}
		if (json.is_array())
			json = json[0];
		for (auto it = json.begin(); it != json.end(); ++it)
		{
			core::UUID uuid{ it.key() };
			const auto& uuidJson = it.value();

			if (!uuidJson.contains("0") || !uuidJson.contains("1"))
				continue;

			AssetInfo info{};
			info.originalPath = std::filesystem::u8path(uuidJson["0"].get<std::string>());
			info.cachePath = std::filesystem::u8path(uuidJson["1"].get<std::string>());

			paths.insert_or_assign(uuid, std::move(info));
		}
		return true;
	}
	SH_EDITOR_API void AssetDatabase::SetProjectDirectory(const std::filesystem::path& dir)
	{
		projectPath = dir;
		libPath = projectPath / "Library";
	}
	SH_EDITOR_API void AssetDatabase::LoadAllAssets(const std::filesystem::path& dir, bool recursive)
	{
		LoadAllAssetsHelper(dir, recursive);

		while (!loadingAssetsQueue.empty())
		{
			AssetLoadData data = std::move(const_cast<AssetLoadData&>(loadingAssetsQueue.top()));
			loadingAssetsQueue.pop();
			ImportAsset(data.path);
		}
		SaveDatabase(project->GetLibraryPath() / "AssetDB.json");
	}

	SH_EDITOR_API bool AssetDatabase::CreateAsset(const std::filesystem::path& dir, const core::SObject& obj)
	{
		if (std::filesystem::exists(dir))
			return false;

		const std::filesystem::path relativePath = std::filesystem::relative(dir, projectPath);
		const std::filesystem::path cachePath = project->GetLibraryPath() / fmt::format("{}.asset", obj.GetUUID().ToString());

		if (!ExportAsset(obj, cachePath))
			return false;

		if (relativePath.empty())
			return false;

		uuids.insert_or_assign(relativePath, obj.GetUUID());
		paths.insert_or_assign(obj.GetUUID(), AssetInfo{relativePath, std::filesystem::relative(cachePath, projectPath)});

		std::ofstream os(dir);
		os << std::setw(4) << obj.Serialize();
		os.close();

		Meta meta{};
		meta.Save(obj, dir);

		return true;
	}

	SH_EDITOR_API void AssetDatabase::AssetWasMoved(const core::UUID& uuid, const std::filesystem::path& newPath)
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return;

		std::filesystem::path relativePath = std::filesystem::relative(newPath, projectPath);

		uuids.erase(it->second.originalPath);
		uuids.insert_or_assign(relativePath, uuid);

		it->second.originalPath = relativePath;

		SaveDatabase(project->GetLibraryPath() / "AssetDB.json");
	}

	SH_EDITOR_API auto AssetDatabase::GetAsset(const core::UUID& uuid) -> std::unique_ptr<core::Asset>
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return nullptr;

		return core::AssetImporter::Load(projectPath / it->second.cachePath);
	}

	SH_EDITOR_API auto AssetDatabase::GetAssetOriginalPath(const core::UUID& uuid) const -> std::optional<std::filesystem::path>
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return std::nullopt;

		return it->second.originalPath;
	}

	SH_EDITOR_API auto AssetDatabase::GetAssetPath(const core::UUID& uuid) const -> const AssetInfo*
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return nullptr;
		return &it->second;
	}

	SH_EDITOR_API auto AssetDatabase::GetAssetUUID(const std::filesystem::path& assetPath) -> std::optional<core::UUID>
	{
		std::filesystem::path relativePath{};
		if (assetPath.is_absolute())
			relativePath = std::filesystem::relative(assetPath, projectPath);
		else
			relativePath = assetPath;

		auto it = uuids.find(relativePath);
		if (it == uuids.end())
			return {};
		return it->second;
	}

	SH_EDITOR_API void AssetDatabase::SetDirty(core::SObject* obj)
	{
		obj->onDestroy.Register(onDestroyListener);
		if(core::IsValid(obj))
			dirtyObjs.push_back(obj);
	}
	SH_EDITOR_API auto AssetDatabase::ExportAsset(const core::SObject& obj, const std::filesystem::path& path, int64_t writeTime) const -> bool
	{
		std::string assetType;

		if (obj.GetType() == render::Texture::GetStaticType())
			assetType = game::TextureAsset::ASSET_NAME;
		else if (obj.GetType() == render::Model::GetStaticType())
			assetType = game::ModelAsset::ASSET_NAME;
		else if (obj.GetType() == game::World::GetStaticType() || obj.GetType() == editor::EditorWorld::GetStaticType())
			assetType = game::WorldAsset::ASSET_NAME;
		else if (obj.GetType() == render::Material::GetStaticType())
			assetType = game::MaterialAsset::ASSET_NAME;
		else if (obj.GetType() == render::Mesh::GetStaticType())
			assetType = game::MeshAsset::ASSET_NAME;
		else if (obj.GetType() == render::Shader::GetStaticType())
			assetType = game::ShaderAsset::ASSET_NAME;
		else
			return false;

		auto asset = core::Factory<core::Asset>::GetInstance()->Create(assetType);
		if (asset == nullptr)
		{
			SH_ERROR_FORMAT("Asset type({}) is invalid", assetType);
			return false;
		}
		asset->SetAsset(obj);
		if (writeTime > 0)
		{
			asset->SetWriteTime(writeTime);
			if (!core::AssetExporter::Save(*asset, path, true))
			{
				SH_ERROR_FORMAT("Asset export failed: {}", path.u8string());
				return false;
			}
		}
		else
		{
			auto writeTime = std::filesystem::file_time_type::clock::now();
			asset->SetWriteTime(writeTime.time_since_epoch().count());
			if (!core::AssetExporter::Save(*asset, path, true))
			{
				SH_ERROR_FORMAT("Asset export failed: {}", path.u8string());
				return false;
			}
			std::filesystem::last_write_time(path, writeTime);
		}
		return true;
	}
	SH_EDITOR_API auto AssetDatabase::IsAssetChanged(const std::filesystem::path& assetPath) -> bool
	{
		const std::filesystem::path metaPath{ GetMetaDirectory(assetPath) };

		Meta meta{};
		if (!meta.Load(metaPath))
			return false;

		auto it = paths.find(meta.GetUUID());
		if (it == paths.end())
			return false;

		const auto& [filePath, assetCachePath] = it->second;
		std::unique_ptr<core::Asset> asset = core::AssetImporter::Load(projectPath / assetCachePath);
		if (asset == nullptr)
			return false;

		const int64_t writeTime = std::filesystem::last_write_time(assetPath).time_since_epoch().count();
		const int64_t assetWriteTime = asset->GetWriteTime();
		const bool bMetaChanged = meta.IsChanged();

		return (assetWriteTime != writeTime) || bMetaChanged;
	}
}//namespace