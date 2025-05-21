#include "AssetDatabase.h"
#include "AssetExtensions.h"
#include "EditorWorld.h"
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "MaterialLoader.h"
#include "Meta.h"

#include "Render/Renderer.h"
#include "Render/Model.h"

#include "Game/World.h"

#include "Core/FileSystem.h"

#include <random>
#include <istream>
#include <ostream>
#include <cassert>
#include <algorithm>
#include <fstream>

namespace sh::editor
{
	core::Observer<false, const core::SObject*>::Listener AssetDatabase::onDestroyListener{ [](const core::SObject* destoryedObj)
		{
			auto it = std::find(AssetDatabase::dirtyObjs.begin(), AssetDatabase::dirtyObjs.end(), destoryedObj);
			if (it != AssetDatabase::dirtyObjs.end())
				AssetDatabase::dirtyObjs.erase(it);
		}
	};
	auto AssetDatabase::CreateMetaDirectory(const std::filesystem::path& assetPath) -> std::filesystem::path
	{
		std::filesystem::path metaPath{ assetPath.parent_path() / assetPath.filename() };
		metaPath += ".meta";
		return metaPath;
	}
	auto AssetDatabase::HasMetaFile(const std::filesystem::path& dir) -> std::optional<std::filesystem::path>
	{
		if (!std::filesystem::exists(dir))
			return std::nullopt;
		std::filesystem::path metaFileDir = CreateMetaDirectory(dir);
		if (!std::filesystem::exists(metaFileDir))
			return std::nullopt;
		return metaFileDir;
	}
	auto AssetDatabase::LoadModel(EditorWorld& world, const std::filesystem::path& dir) -> render::Model*
	{
		std::filesystem::path metaDir{ CreateMetaDirectory(dir) };

		MeshImporter importer{};
		Meta meta{};
		if (meta.Load(metaDir))
			meta.LoadImporter(importer);

		static ModelLoader loader{ *world.renderer.GetContext()};
		render::Model* modelPtr = nullptr;
		if (dir.extension() == ".obj")
			modelPtr = loader.Load(dir);
		else
			modelPtr = loader.LoadGLTF(dir);

		if (modelPtr == nullptr)
			return nullptr;

		world.models.AddResource(dir.u8string(), modelPtr);

		if (meta.IsLoad())
			meta.LoadSObject(*modelPtr);
		else
			meta.Save(*modelPtr, importer, metaDir);

		uuids.insert_or_assign(dir, modelPtr->GetUUID());
		paths.insert_or_assign(modelPtr->GetUUID(), dir);

		return modelPtr;
	}
	auto AssetDatabase::LoadTexture(EditorWorld& world, const std::filesystem::path& dir) -> render::Texture*
	{
		std::filesystem::path metaDir{ CreateMetaDirectory(dir) };

		TextureImporter importer{};

		Meta meta{};
		if (meta.Load(metaDir))
			meta.LoadImporter(importer);

		static TextureLoader loader{ *world.renderer.GetContext() };
		auto ptr = loader.Load(dir.string(), importer);
		if (ptr == nullptr)
			return nullptr;

		world.textures.AddResource(dir.u8string(), ptr);
		ptr->SetName(dir.stem().u8string());

		if (meta.IsLoad())
			meta.LoadSObject(*ptr);
		else
			meta.Save(*ptr, importer, metaDir);

		uuids.insert_or_assign(dir, ptr->GetUUID());
		paths.insert_or_assign(ptr->GetUUID(), dir);

		return ptr;
	}
	auto AssetDatabase::LoadMaterial(EditorWorld& world, const std::filesystem::path& dir) -> render::Material*
	{
		static MaterialLoader loader{ *world.renderer.GetContext() };
		
		auto ptr = loader.Load(dir);
		if (ptr == nullptr)
			return nullptr;

		world.materials.AddResource(dir.u8string(), ptr);
		ptr->SetName(dir.stem().string());

		uuids.insert_or_assign(dir, ptr->GetUUID());
		paths.insert_or_assign(ptr->GetUUID(), dir);
		return ptr;
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

	SH_EDITOR_API auto AssetDatabase::ImportAsset(EditorWorld& world, const std::filesystem::path& dir) -> core::SObject*
	{
		if (!std::filesystem::exists(dir))
			return nullptr;
		if (std::filesystem::is_directory(dir))
			return nullptr;
		
		std::string extension = dir.extension().string();

		auto type = AssetExtensions::CheckType(extension);
		if (type == AssetExtensions::Type::Texture)
			return LoadTexture(world, dir);
		if (type == AssetExtensions::Type::Model)
			return LoadModel(world, dir);
		if (type == AssetExtensions::Type::Material)
			return LoadMaterial(world, dir);
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
			const auto& assetPath = it->second;

			if (obj->GetType() == render::Material::GetStaticType())
			{
				SaveMaterial(static_cast<render::Material*>(obj), assetPath);
			}
			else if (obj->GetType() == render::Texture::GetStaticType())
			{
				render::Texture* texture = reinterpret_cast<render::Texture*>(obj);
				TextureImporter importer{};
				importer.bSRGB = texture->IsSRGB();
				importer.aniso = texture->GetAnisoLevel();
				Meta meta{};
				meta.Save(*texture, importer, CreateMetaDirectory(assetPath));
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
	SH_EDITOR_API void AssetDatabase::LoadAllAssets(EditorWorld& world, const std::filesystem::path& dir, bool recursive)
	{
		LoadAllAssetsHelper(dir, recursive);
		while (!loadingAssetsQueue.empty())
		{
			AssetLoadData data = std::move(const_cast<AssetLoadData&>(loadingAssetsQueue.top()));
			loadingAssetsQueue.pop();
			ImportAsset(world, data.path);
		}
	}

	SH_EDITOR_API bool AssetDatabase::CreateAsset(EditorWorld& world, const std::filesystem::path& dir, const core::ISerializable& serializable)
	{
		if (std::filesystem::exists(dir))
			return false;

		auto json = serializable.Serialize();
		if (json.contains("uuid"))
			uuids.insert_or_assign(dir, core::UUID(json["uuid"].get<std::string>()));

		std::ofstream os(dir);
		os << std::setw(4) << json;
		os.close();

		return true;
	}

	SH_EDITOR_API auto AssetDatabase::GetAssetUUID(const std::filesystem::path& dir) -> std::optional<core::UUID>
	{
		auto it = uuids.find(dir);
		if (it == uuids.end())
			return std::nullopt;
		return it->second;
	}

	SH_EDITOR_API void AssetDatabase::SetDirty(core::SObject* obj)
	{
		obj->onDestroy.Register(onDestroyListener);
		if(core::IsValid(obj))
			dirtyObjs.push_back(obj);
	}
}//namespace