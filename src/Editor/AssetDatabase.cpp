#include "AssetDatabase.h"
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "MaterialLoader.h"

#include "Render/Renderer.h"

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

	void AssetDatabase::CreateMeta(core::SObject* ptr, const std::filesystem::path& metaDir)
	{
		core::Json metaJson = ptr->Serialize();
		std::ofstream os{ metaDir.string() };
		if (os.is_open())
		{
			os << std::setw(4) << metaJson;
			os.close();
		}
		else
		{
			SH_ERROR_FORMAT("Can't create meta: {}", metaDir.string());
		}
	}
	void AssetDatabase::CreateOrLoadMeta(core::SObject* ptr, const std::filesystem::path& metaDir)
	{
		if (std::filesystem::exists(metaDir))
		{
			auto file = core::FileSystem::LoadText(metaDir);
			if (!file)
			{
				SH_ERROR_FORMAT("Can't load file: {}", metaDir.string());
				return;
			}
			if (file.value().empty())
			{
				CreateMeta(ptr, metaDir);
				return;
			}
			core::Json metaJson{ core::Json::parse(file.value()) };
			ptr->Deserialize(metaJson);
			assert(ptr->GetUUID().ToString() == metaJson["uuid"].get<std::string>());
		}
		else
		{
			CreateMeta(ptr, metaDir);
		}
	}

	auto AssetDatabase::LoadMesh(game::World& world, const std::filesystem::path& dir, const std::filesystem::path& metaDir) -> render::Mesh*
	{
		static ModelLoader loader{ *world.renderer.GetContext()};
		auto ptr = loader.Load(dir.string());
		if (ptr == nullptr)
			return nullptr;

		world.meshes.AddResource(dir.string(), ptr);
		ptr->SetName(dir.stem().string());

		CreateOrLoadMeta(ptr, metaDir);
		uuids.insert_or_assign(dir, ptr->GetUUID());
		paths.insert_or_assign(ptr->GetUUID().ToString(), dir);

		return ptr;
	}
	auto AssetDatabase::LoadTexture(game::World& world, const std::filesystem::path& dir, const std::filesystem::path& metaDir) -> render::Texture*
	{
		static TextureLoader loader{ *world.renderer.GetContext() };
		auto ptr = loader.Load(dir.string());
		if (ptr == nullptr)
			return nullptr;

		world.textures.AddResource(dir.string(), ptr);
		ptr->SetName(dir.stem().u8string());

		CreateOrLoadMeta(ptr, metaDir);
		uuids.insert_or_assign(dir, ptr->GetUUID());
		paths.insert_or_assign(ptr->GetUUID().ToString(), dir);

		return ptr;
	}
	auto AssetDatabase::LoadMaterial(game::World& world, const std::filesystem::path& dir) -> render::Material*
	{
		static MaterialLoader loader{ *world.renderer.GetContext() };
		
		auto ptr = loader.Load(dir.string());
		if (ptr == nullptr)
			return nullptr;

		world.materials.AddResource(dir.string(), ptr);
		ptr->SetName(dir.stem().string());

		uuids.insert_or_assign(dir, ptr->GetUUID());
		paths.insert_or_assign(ptr->GetUUID().ToString(), dir);
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

	SH_EDITOR_API auto AssetDatabase::ImportAsset(game::World& world, const std::filesystem::path& dir) -> core::SObject*
	{
		if (!std::filesystem::exists(dir))
			return nullptr;
		if (std::filesystem::is_directory(dir))
			return nullptr;
		
		std::string filename = dir.filename().string() + ".meta";
		std::string extension = dir.extension().string();
		auto parentDir = dir.parent_path();

		auto metaDir = parentDir / filename;
		bool hasMetaFile = std::filesystem::exists(metaDir);

		if (extension == ".jpg" || extension == ".png")
			return LoadTexture(world, dir, metaDir);
		if (extension == ".obj")
			return LoadMesh(world, dir, metaDir);
		if (extension == ".mat")
			return LoadMaterial(world, dir);
		return nullptr;
	}

	SH_EDITOR_API void AssetDatabase::SaveAllAssets()
	{
		for (auto obj : dirtyObjs)
		{
			auto it = paths.find(obj->GetUUID().ToString());
			if (it == paths.end())
				continue;
			if (obj->GetType() == render::Material::GetStaticType())
			{
				SaveMaterial(static_cast<render::Material*>(obj), it->second);
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
	SH_EDITOR_API void AssetDatabase::LoadAllAssets(game::World& world, const std::filesystem::path& dir, bool recursive)
	{
		LoadAllAssetsHelper(dir, recursive);
		while (!loadingAssetsQueue.empty())
		{
			AssetLoadData data = std::move(const_cast<AssetLoadData&>(loadingAssetsQueue.top()));
			loadingAssetsQueue.pop();
			ImportAsset(world, data.path);
		}
	}

	SH_EDITOR_API bool AssetDatabase::CreateAsset(game::World& world, const std::filesystem::path& dir, const core::ISerializable& serializable)
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