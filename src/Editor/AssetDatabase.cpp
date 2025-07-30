#include "AssetDatabase.h"
#include "AssetExtensions.h"
#include "Meta.h"
#include "UI/Project.h"

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
#include "Game/MaterialLoader.h"
#include "Game/ShaderLoader.h"
#include "Game/WorldLoader.h"
#include "Game/AssetLoaderFactory.h"

#include <random>
#include <istream>
#include <ostream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <cstdint>
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
	auto AssetDatabase::LoadMaterial(const std::filesystem::path& dir) -> render::Material*
	{
		if (project == nullptr)
			return nullptr;

		static game::MaterialLoader loader{ *project->renderer.GetContext() };
		
		auto matPtr = loader.Load(dir);
		if (matPtr == nullptr)
			return nullptr;

		std::filesystem::path relativePath{ std::filesystem::relative(dir, projectPath) };

		project->loadedAssets.AddResource(matPtr->GetUUID(), matPtr);
		matPtr->SetName(dir.stem().string());

		uuids.insert_or_assign(relativePath, matPtr->GetUUID());
		paths.insert_or_assign(matPtr->GetUUID(), AssetInfo{ relativePath, relativePath });

		Meta meta{};
		meta.Save(*matPtr, GetMetaDirectory(projectPath / relativePath));
		return matPtr;
	}
	void AssetDatabase::SaveMaterial(render::Material* mat, const std::filesystem::path& dir)
	{
		if (!core::IsValid(mat))
			return;

		core::Json matJson{ mat->Serialize() };

		std::ofstream os{ dir };
		os << std::setw(4) << matJson;
		os.close();
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
			render::Model* modelPtr = static_cast<render::Model*>(LoadAsset(dir, loader, true));
			if (modelPtr != nullptr)
				project->loadedAssets.AddResource(modelPtr->GetUUID(), modelPtr);
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
				render::Shader* shaderPtr = static_cast<render::Shader*>(LoadAsset(dir, loader, true));
				if (shaderPtr != nullptr)
					project->loadedAssets.AddResource(shaderPtr->GetUUID(), shaderPtr);
				return shaderPtr;
			}
		}
		if (type == AssetExtensions::Type::Material)
			return LoadMaterial(dir);
		if (type == AssetExtensions::Type::World)
		{
			static game::WorldLoader loader{ project->renderer, project->gui };
			game::World* worldPtr = static_cast<game::World*>(LoadAsset(dir, loader, false));
			if (worldPtr != nullptr)
				game::GameManager::GetInstance()->AddWorld(*worldPtr);
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
		int64_t writeTime = std::filesystem::last_write_time(path).time_since_epoch().count();

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
						SH_INFO_FORMAT("Asset {} was changed!", relativePath.u8string());
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

		auto asset = core::Factory<core::Asset>::GetInstance()->Create(loader.GetAssetName());
		if (asset == nullptr)
		{
			SH_ERROR_FORMAT("Asset type({}) is invalid", loader.GetAssetName());
			return nullptr;
		}

		asset->SetAsset(*objPtr);
		asset->SetWriteTime(writeTime);

		std::filesystem::path cachePath{ libPath / fmt::format("{}.asset", objPtr->GetUUID().ToString()) };
		if (!core::AssetExporter::Save(*asset, cachePath, true))
		{
			SH_ERROR_FORMAT("Asset export failed: {}", cachePath.u8string());
			return nullptr;
		}

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
	}

	SH_EDITOR_API bool AssetDatabase::CreateAsset(const std::filesystem::path& dir, const core::ISerializable& serializable)
	{
		if (std::filesystem::exists(dir))
			return false;

		auto json = serializable.Serialize();
		if (json.contains("uuid"))
		{
			std::filesystem::path relativePath = std::filesystem::relative(dir, projectPath);
			if (relativePath.empty())
				return false;

			const core::UUID uuid{ json["uuid"].get<std::string>() };
			uuids.insert_or_assign(relativePath, uuid);
			paths.insert_or_assign(uuid, AssetInfo{ relativePath, relativePath });
		}

		std::ofstream os(dir);
		os << std::setw(4) << json;
		os.close();

		return true;
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
}//namespace