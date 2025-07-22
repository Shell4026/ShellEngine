#pragma once
#include "Export.h"
#include "UUID.h"
#include "Asset.h"

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
	class AssetBundle
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

        SH_CORE_API void Clear();

        SH_CORE_API auto AddAsset(const Asset& asset, bool bCompress) -> bool;

        SH_CORE_API auto HasAsset(const UUID& uuid) const -> bool;

        SH_CORE_API auto GetAllAssetUUIDs() const -> std::vector<UUID>;

        SH_CORE_API auto SaveBundle(const std::filesystem::path& bundlePath) const -> bool;
        SH_CORE_API auto LoadBundle(const std::filesystem::path& bundlePath) -> bool;

        SH_CORE_API auto LoadAsset(const core::UUID& uuid) -> std::unique_ptr<Asset>;

        SH_CORE_API auto GetVersion() const -> uint32_t;
	};
}//namespace