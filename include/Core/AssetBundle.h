#pragma once
#include "Export.h"
#include "UUID.h"
#include "Asset.h"
#include "NonCopyable.h"

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <array>
#include <filesystem>
#include <memory>
#include <fstream>
namespace sh::core
{
	/// @brief 에셋들의 묶음을 나타내는 클래스
	class AssetBundle : core::INonCopyable
	{
    private:
        struct Header
        {
            uint32_t version;
            uint32_t numAssets;
        } header;
        struct AssetEntry
        {
            std::array<uint32_t, 4> uuid;
            char type[4];
            uint64_t dataOffset;
            uint64_t dataSize;
            bool bCompressed;
        };
        std::filesystem::path bundlePath;

        std::unordered_map<UUID, AssetEntry> assetEntries;
        std::vector<uint8_t> bundleData;
        
        std::size_t nextAssetDataOffset = 0;

        mutable std::fstream bundleStream;
    public:
        constexpr static uint32_t VERSION = 1;
    public:
        SH_CORE_API AssetBundle();
        SH_CORE_API AssetBundle(AssetBundle&& other) noexcept;

        /// @brief 번들이 열려있다면 닫고, 아니라면 데이터를 초기화 한다.
        SH_CORE_API void Clear();

        /// @brief 번들에 에셋을 추가한다.
        /// @param asset 에셋
        /// @param bCompress 압축 여부
        /// @return 성공 시 true, 실패 시 false
        SH_CORE_API auto AddAsset(const Asset& asset, bool bCompress) -> bool;

        /// @brief 열린 번들이 해당 UUID의 에셋을 포함하고 있는지 반환한다.
        /// @param uuid 에셋 UUID
        /// @return 있다면 true, 없다면 false
        SH_CORE_API auto HasAsset(const UUID& uuid) const -> bool;
        /// @brief 번들 파일이 열렸는지 반환 한다.
        /// @return 열려있다면 true, 아니라면 false
        SH_CORE_API auto IsLoaded() const -> bool;

        /// @brief 해당 번들이 가지고 있는 모든 에셋들의 UUID를 반환한다.
        /// @return 에셋 UUID 벡터
        SH_CORE_API auto GetAllAssetUUIDs() const -> std::vector<UUID>;

        /// @brief 번들 파일을 저장하고 번들을 닫는다.
        /// @param bundlePath 경로
        /// @return 성공 시 true, 실패 시 false
        SH_CORE_API auto SaveBundle(const std::filesystem::path& bundlePath) const -> bool;
        /// @brief 번들 파일을 불러온다. 에셋은 메모리에 올라오지 않는다.
        /// @param bundlePath 경로
        SH_CORE_API auto LoadBundle(const std::filesystem::path& bundlePath) -> bool;

        /// @brief 열린 번들에서 에셋을 메모리로 불러온다.
        /// @param uuid 에셋 UUID
        /// @return 실패 시 nullptr 반환
        SH_CORE_API auto LoadAsset(const core::UUID& uuid) -> std::unique_ptr<Asset>;

        /// @brief 열린 번들의 버전을 반환 한다.
        /// @return 번들 버전
        SH_CORE_API auto GetVersion() const -> uint32_t;

        SH_CORE_API auto GetAssetEntries() const -> const std::unordered_map<UUID, AssetEntry>&;
	};
}//namespace