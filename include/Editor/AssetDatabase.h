#pragma once
#include "Export.h"

#include "Core/ISerializable.h"
#include "Core/SContainer.hpp"
#include "Core/UUID.h"
#include "Core/Observer.hpp"

#include <filesystem>
#include <optional>
#include <queue>

namespace sh::core
{
	class SObject;
}
namespace sh::render
{
	class Texture;
	class Mesh;
	class Material;
}
namespace sh::game
{
	class World;
}

namespace sh::editor
{
	class AssetDatabase
	{
	private:
		struct AssetLoadData
		{
			int priority = 0;
			std::filesystem::path path;

			AssetLoadData(int priority, const std::filesystem::path& path) :
				priority(priority), path(path)
			{}
			AssetLoadData(const AssetLoadData& other) :
				priority(other.priority), path(other.path)
			{}
			AssetLoadData(AssetLoadData&& other) noexcept :
				priority(other.priority), path(std::move(other.path))
			{}

			bool operator<(const AssetLoadData& other) const
			{
				return priority < other.priority;
			}
			auto operator=(AssetLoadData&& other) noexcept -> AssetLoadData&
			{
				priority = other.priority;
				path = std::move(other.path);
				return *this;
			}
			auto operator=(const AssetLoadData& other) -> AssetLoadData&
			{
				priority = other.priority;
				path = other.path;
				return *this;
			}
		};
		SH_EDITOR_API inline static core::SHashMap<std::filesystem::path, core::UUID> uuids{};
		SH_EDITOR_API inline static core::SHashMap<std::string, std::filesystem::path> paths{};
		SH_EDITOR_API inline static core::SVector<core::SObject*> dirtyObjs{};
		SH_EDITOR_API inline static std::priority_queue<AssetLoadData> loadingAssetsQueue{};
		SH_EDITOR_API static core::Observer<false, core::SObject*>::Listener onDestroyListener;
	private:
		static void CreateMeta(core::SObject* ptr, const std::filesystem::path& metaDir);
		static void CreateOrLoadMeta(core::SObject* ptr, const std::filesystem::path& metaDir);
		static auto LoadMesh(game::World& world, const std::filesystem::path& dir, const std::filesystem::path& metaDir) -> render::Mesh*;
		static auto LoadTexture(game::World& world, const std::filesystem::path& dir, const std::filesystem::path& metaDir) -> render::Texture*;
		static auto LoadMaterial(game::World& world, const std::filesystem::path& dir) -> render::Material*;
		static void SaveMaterial(render::Material* mat, const std::filesystem::path& dir);
		static void LoadAllAssetsHelper(const std::filesystem::path& dir, bool recursive);
	public:
		/// @brief 변경 사항이 있는 에셋을 모두 저장하는 함수
		/// @brief SetDirty()를 통해 알려줘야 한다.
		SH_EDITOR_API static void SaveAllAssets();
		/// @brief 해당 경로에 있는 에셋을 모두 불러오는 함수
		/// @param dir 경로
		/// @param recursive 하위 경로도 포함 할 것인지
		SH_EDITOR_API static void LoadAllAssets(game::World& world, const std::filesystem::path& dir, bool recursive);
		SH_EDITOR_API static auto ImportAsset(game::World& world, const std::filesystem::path& dir) -> core::SObject*;
		SH_EDITOR_API static bool CreateAsset(game::World& world, const std::filesystem::path& dir, const core::ISerializable& serializable);
		SH_EDITOR_API static auto GetAssetUUID(const std::filesystem::path& dir) -> std::optional<core::UUID>;
		/// @brief 에셋에 변경 사항이 존재한다고 알리는 함수
		/// @param obj 포인터
		SH_EDITOR_API static void SetDirty(core::SObject* obj);
	};
}//namespace