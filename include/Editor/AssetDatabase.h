#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/ISerializable.h"
#include "Core/IAssetLoader.h"
#include "Core/SContainer.hpp"
#include "Core/UUID.h"
#include "Core/Observer.hpp"

#include <filesystem>
#include <optional>
#include <queue>
#include <optional>
namespace sh::core
{
	class SObject;
}
namespace sh::render
{
	class Texture;
	class Mesh;
	class Model;
	class Material;
}
namespace sh::game
{
	class World;
}

namespace sh::editor
{
	class Project;
	class AssetDatabase : public core::Singleton<AssetDatabase>
	{
		friend core::Singleton<AssetDatabase>;
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
		struct AssetInfo
		{
			std::filesystem::path originalPath;
			std::filesystem::path cachePath;
		};
		Project* project = nullptr;

		std::filesystem::path projectPath;
		std::filesystem::path libPath;

		std::unordered_map<std::filesystem::path, core::UUID> uuids{};
		std::unordered_map<core::UUID, AssetInfo> paths{};
		core::SVector<core::SObject*> dirtyObjs{};
		std::priority_queue<AssetLoadData> loadingAssetsQueue{};

		core::Observer<false, const core::SObject*>::Listener onDestroyListener;
	private:
		auto GetMetaDirectory(const std::filesystem::path& assetPath) -> std::filesystem::path;
		/// @brief 파일의 메타 파일이 존재 하는지?
		/// @param dir 파일 경로
		/// @return 있다면 메타 파일의 경로를 반환, 없다면 nullopt 반환
		auto HasMetaFile(const std::filesystem::path& dir) -> std::optional<std::filesystem::path>;
		auto LoadMaterial(const std::filesystem::path& dir) -> render::Material*;
		void SaveMaterial(render::Material* mat, const std::filesystem::path& dir);
		void LoadAllAssetsHelper(const std::filesystem::path& dir, bool recursive);
		auto LoadAsset(const std::filesystem::path& path, core::IAssetLoader& loader, bool bMetaSaveWithObj) -> core::SObject*;
	protected:
		SH_EDITOR_API AssetDatabase();
	public:
		SH_EDITOR_API void SetProject(Project& project);

		SH_EDITOR_API void SaveDatabase(const std::filesystem::path& dir);
		SH_EDITOR_API auto LoadDatabase(const std::filesystem::path& dir) -> bool;

		SH_EDITOR_API void SetProjectDirectory(const std::filesystem::path& dir);
		/// @brief 변경 사항이 있는 에셋을 모두 저장하는 함수
		/// @brief SetDirty()를 통해 알려줘야 한다.
		SH_EDITOR_API void SaveAllAssets();
		/// @brief 해당 경로에 있는 에셋을 모두 불러오는 함수
		/// @param dir 경로
		/// @param recursive 하위 경로도 포함 할 것인지
		SH_EDITOR_API void LoadAllAssets(const std::filesystem::path& dir, bool recursive);
		SH_EDITOR_API auto ImportAsset(const std::filesystem::path& dir) -> core::SObject*;
		SH_EDITOR_API bool CreateAsset(const std::filesystem::path& dir, const core::ISerializable& serializable);
		SH_EDITOR_API auto GetAsset(const core::UUID& uuid) -> std::unique_ptr<core::Asset>;
		SH_EDITOR_API auto GetAssetOriginalPath(const core::UUID& uuid) const -> std::optional<std::filesystem::path>;
		/// @brief 해당 경로의 파일의 에셋의 UUID를 반환한다.
		/// @param assetPath 에셋 경로
		/// @return 에셋이 로드 돼 있지 않다면 nullopt반환, 로드 돼 있다면 UUID를 반환
		SH_EDITOR_API auto GetAssetUUID(const std::filesystem::path& assetPath) -> std::optional<core::UUID>;
		/// @brief 에셋에 변경 사항이 존재한다고 알리는 함수
		/// @param obj 포인터
		SH_EDITOR_API void SetDirty(core::SObject* obj);
	};
}//namespace