#include "AssetDatabase.h"
#include "AssetExtensions.h"
#include "Meta.h"
#include "EditorWorld.h"

#include "Core/FileSystem.h"
#include "Core/Asset.h"
#include "Core/AssetImporter.h"
#include "Core/AssetExporter.h"
#include "Core/Logger.h"

#include "Render/Renderer.h"
#include "Render/Model.h"
#include "Render/VulkanImpl/VulkanShaderPassBuilder.h"
#include "Render/VulkanImpl/VulkanContext.h"

#include "Game/World.h"
#include "Game/Prefab.h"

#include "Game/Asset/TextureLoader.h"
#include "Game/Asset/ModelLoader.h"
#include "Game/Asset/MaterialLoader.h"
#include "Game/Asset/ShaderLoader.h"
#include "Game/Asset/WorldLoader.h"
#include "Game/Asset/PrefabLoader.h"
#include "Game/Asset/BinaryLoader.h"
#include "Game/Asset/TextLoader.h"
#include "Game/Asset/FontLoader.h"
#include "Game/Asset/ScriptableObjectLoader.h"

#include "Game/Asset/TextureAsset.h"
#include "Game/Asset/ModelAsset.h"
#include "Game/Asset/MeshAsset.h"
#include "Game/Asset/MaterialAsset.h"
#include "Game/Asset/ShaderAsset.h"
#include "Game/Asset/WorldAsset.h"
#include "Game/Asset/PrefabAsset.h"
#include "Game/Asset/BinaryAsset.h"
#include "Game/Asset/TextAsset.h"
#include "Game/Asset/FontAsset.h"
#include "Game/Asset/ScriptableObjectAsset.h"

#include <istream>
#include <ostream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <chrono>
namespace sh::editor
{
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

	SH_EDITOR_API void AssetDatabase::Init(const std::filesystem::path& projectPath, const render::IRenderContext& ctx, const game::ImGUImpl& imgui)
	{
		this->projectPath = projectPath;
		this->guiCtx = &imgui;
		libPath = projectPath / "Library";

		assert(project->renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan); // TODO
		static render::vk::VulkanShaderPassBuilder passBuilder{ static_cast<const render::vk::VulkanContext&>(ctx) };
		auto shaderLoader = std::make_unique<game::ShaderLoader>(&passBuilder);
		shaderLoader->SetCachePath(projectPath / "temp");

		assetLoaders.Clear();
		assetLoaders.RegisterLoader(AssetExtensions::Type::Model, std::make_unique<game::ModelLoader>(ctx), 2, true);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Texture, std::make_unique<game::TextureLoader>(ctx), 2, true);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Shader, std::move(shaderLoader), 2, false);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Binary, std::make_unique<game::BinaryLoader>(), 2, false);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Text, std::make_unique<game::TextLoader>(), 2, false);

		assetLoaders.RegisterLoader(AssetExtensions::Type::Material, std::make_unique<game::MaterialLoader>(ctx), 1, false);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Font, std::make_unique<game::FontLoader>(), 1, false);
		assetLoaders.RegisterLoader(AssetExtensions::Type::Prefab, std::make_unique<game::PrefabLoader>(), 1, false);

		assetLoaders.RegisterLoader(AssetExtensions::Type::World, std::make_unique<game::WorldLoader>(), 0, false);
		assetLoaders.RegisterLoader(AssetExtensions::Type::ScriptableObject, std::make_unique<game::ScriptableObjectLoader>(), -1, false);
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

		core::Json json = core::Json::parse(opt.value());
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
			uuids.insert_or_assign(info.originalPath, uuid);
		}
		return true;
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
			const uint64_t writeTime = std::filesystem::last_write_time(projectPath / originalPath).time_since_epoch().count();
			const std::string ext = originalPath.extension().u8string();
			const AssetExtensions::Type type = AssetExtensions::CheckType(ext);

			const AssetLoaderRegistry::Importer* const importer = assetLoaders.GetLoader(type);
			if (importer->bObjDataInMeta)
			{
				Meta meta{};
				meta.SaveWithObj(*obj, Meta::CreateMetaDirectory(projectPath / originalPath), false);
			}
			else
			{
				std::ofstream os{ projectPath / originalPath };
				os << std::setw(4) << obj->Serialize();
				os.close();

				Meta meta{};
				meta.Save(*obj, Meta::CreateMetaDirectory(projectPath / originalPath), false);
			}
			ExportAsset(*obj, projectPath / cachePath, writeTime);
		}
		dirtyObjs.clear();
	}
	SH_EDITOR_API void AssetDatabase::LoadAllAssets(const std::filesystem::path& dir, bool recursive, bool bOverLoad)
	{
		LoadAllAssetsHelper(dir, recursive);

		while (!loadingAssetsQueue.empty())
		{
			AssetLoadData data = std::move(const_cast<AssetLoadData&>(loadingAssetsQueue.top()));
			loadingAssetsQueue.pop();

			ImportAsset(data.path, bOverLoad);
		}
		SaveDatabase(libPath / "AssetDB.json");
	}
	SH_EDITOR_API auto AssetDatabase::ImportAsset(const std::filesystem::path& dir, bool bOverLoad) -> core::SObject*
	{
		std::filesystem::path abPath = dir;
		if (dir.is_relative())
			abPath = projectPath / abPath;

		if (!bOverLoad)
		{
			auto uuidOpt = GetAssetUUID(abPath);
			if (uuidOpt.has_value())
			{
				core::SObject* const sobjPtr = core::SObjectManager::GetInstance()->GetSObject(uuidOpt.value());
				if (sobjPtr != nullptr)
					return sobjPtr;
			}
		}

		if (!std::filesystem::exists(abPath))
			return nullptr;
		if (std::filesystem::is_directory(abPath))
			return nullptr;

		const std::string extension = abPath.extension().string();

		const auto type = AssetExtensions::CheckType(extension);
		const AssetLoaderRegistry::Importer* const importerPtr = assetLoaders.GetLoader(type);
		if (importerPtr == nullptr)
			return nullptr;

		const bool bAssetChanged = IsAssetChanged(abPath);

		core::SObject* const objPtr = LoadAsset(abPath, *importerPtr->loader, importerPtr->bObjDataInMeta);

		// 모델 파일은 특수처리 필요
		if (type == AssetExtensions::Type::Model)
		{
			if (objPtr != nullptr && bAssetChanged)
			{
				render::Model* const modelPtr = static_cast<render::Model*>(objPtr);
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
		if (objPtr != nullptr)
			onAssetImported.Notify(objPtr);

		return objPtr;
	}

	SH_EDITOR_API bool AssetDatabase::CreateAsset(const std::filesystem::path& dir, const core::SObject& obj)
	{
		if (std::filesystem::exists(dir))
			return false;

		const std::filesystem::path relativePath = std::filesystem::relative(dir, projectPath);
		const std::filesystem::path cachePath = libPath / fmt::format("{}.asset", obj.GetUUID().ToString());

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
		meta.Save(obj, Meta::CreateMetaDirectory(dir));

		onAssetImported.Notify(const_cast<core::SObject*>(&obj));

		return true;
	}

	SH_EDITOR_API void AssetDatabase::AssetWasMoved(const core::UUID& uuid, const std::filesystem::path& newPath)
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return;

		std::filesystem::path relativePath = std::filesystem::relative(newPath, projectPath);

		uuids.erase(std::filesystem::relative(it->second.originalPath));
		uuids.insert_or_assign(relativePath, uuid);

		it->second.originalPath = relativePath;

		SaveDatabase(libPath / "AssetDB.json");
	}

	SH_EDITOR_API void AssetDatabase::DeleteAsset(const core::UUID& uuid)
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return;

		const auto& filePath = projectPath / it->second.originalPath;
		const auto& cachePath = projectPath / it->second.originalPath;
		if(std::filesystem::exists(filePath))
			std::filesystem::remove(filePath);
		if (std::filesystem::exists(cachePath))
			std::filesystem::remove(cachePath);
		const auto metaPath = Meta::CreateMetaDirectory(projectPath / it->second.originalPath);
		if (std::filesystem::exists(metaPath))
			std::filesystem::remove(metaPath);

		uuids.erase(it->second.originalPath);
		paths.erase(it);

		auto objPtr = core::SObjectManager::GetInstance()->GetSObject(uuid);
		if (objPtr != nullptr)
		{
			objPtr->Destroy();
			onAssetRemoved.Notify(uuid);
		}

		SaveDatabase(libPath / "AssetDB.json");
	}

	SH_EDITOR_API void AssetDatabase::MoveAssetToDirectory(const core::UUID& uuid, const std::filesystem::path& directoryPath)
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return;

		std::filesystem::path newPath;
		if (directoryPath.is_relative())
		{
			if (!std::filesystem::exists(projectPath / directoryPath) || !std::filesystem::is_directory(projectPath / directoryPath))
				return;
			newPath = projectPath / "Assets" / directoryPath / it->second.originalPath.filename();
		}
		else
		{
			if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath))
				return;
			newPath = directoryPath / it->second.originalPath.filename();
		}

		const auto& filePath = projectPath / it->second.originalPath;

		std::filesystem::rename(filePath, newPath);
		const auto metaPath = Meta::CreateMetaDirectory(filePath);
		if (std::filesystem::exists(metaPath))
			std::filesystem::rename(metaPath, Meta::CreateMetaDirectory(newPath));

		AssetWasMoved(uuid, newPath);
	}

	SH_EDITOR_API auto AssetDatabase::GetAsset(const core::UUID& uuid) -> std::unique_ptr<core::Asset>
	{
		auto it = paths.find(uuid);
		if (it == paths.end())
			return nullptr;

		return core::AssetImporter::Load(projectPath / it->second.cachePath);
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
		else if (obj.GetType() == game::Prefab::GetStaticType())
			assetType = game::PrefabAsset::ASSET_NAME;
		else if (obj.GetType() == game::BinaryObject::GetStaticType())
			assetType = game::BinaryAsset::ASSET_NAME;
		else if (obj.GetType() == game::TextObject::GetStaticType())
			assetType = game::TextAsset::ASSET_NAME;
		else if (obj.GetType() == render::Font::GetStaticType())
			assetType = game::FontAsset::ASSET_NAME;
		else if (obj.GetType().IsChildOf(game::ScriptableObject::GetStaticType()))
			assetType = game::ScriptableObjectAsset::ASSET_NAME;
		else
		{
			SH_ERROR_FORMAT("Failed to export asset (type: {})", obj.GetType().name.ToString());
			return false;
		}

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
		const std::filesystem::path metaPath{ Meta::CreateMetaDirectory(assetPath) };

		// 처음 보는 경로의 에셋인 경우에도 변경으로 간주한다.
		if (uuids.find(std::filesystem::relative(assetPath, projectPath)) == uuids.end())
			return true;

		Meta meta{};
		// 메타 파일이 없는 경우에도 변경으로 친다.
		if (!meta.Load(metaPath))
			return true;

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
	SH_EDITOR_API auto AssetDatabase::GetAssetImporter(AssetExtensions::Type type) const -> const AssetLoaderRegistry::Importer*
	{
		return assetLoaders.GetLoader(type);
	}
	auto AssetDatabase::HasMetaFile(const std::filesystem::path& dir) -> std::optional<std::filesystem::path>
	{
		if (!std::filesystem::exists(dir))
			return std::nullopt;
		std::filesystem::path metaFileDir = Meta::CreateMetaDirectory(dir);
		if (!std::filesystem::exists(metaFileDir))
			return std::nullopt;
		return metaFileDir;
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
				const std::string extension = entry.path().extension().u8string();
				if (extension == ".meta")
					continue;

				int priority = 0;
				const auto type = AssetExtensions::CheckType(extension);
				const auto importer = assetLoaders.GetLoader(type);
				if (importer != nullptr)
					priority = importer->priority;
				loadingAssetsQueue.push(AssetLoadData{ priority, entry.path() });
			}
		}
	}
	auto AssetDatabase::LoadAsset(const std::filesystem::path& path, core::IAssetLoader& loader, bool bMetaSaveWithObj) -> core::SObject*
	{
		std::filesystem::path metaDir{ Meta::CreateMetaDirectory(path) };
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
}//namespace