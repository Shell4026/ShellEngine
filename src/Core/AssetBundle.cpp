#include "AssetBundle.h"
#include "AssetExporter.h"
#include "AssetImporter.h"
#include "Logger.h"
namespace sh::core
{
	AssetBundle::AssetBundle()
	{
		static_assert(sizeof(Header) == 8);
		static_assert(sizeof(AssetEntry) == 48);
	}
	SH_CORE_API void AssetBundle::Clear()
	{
		bundleStream.clear();
		bundleStream.close();

		assetEntries.clear();
		bundleData.clear();
		nextAssetDataOffset = 0;
	}
	SH_CORE_API auto AssetBundle::AddAsset(const Asset& asset, bool bCompress) -> bool
	{
		if (asset.IsEmpty())
			return false;
		if (assetEntries.find(asset.GetUUID()) != assetEntries.end())
			return false;

		std::size_t lastOffset = bundleData.size();

		std::vector<uint8_t> assetData = AssetExporter::SaveToMemory(asset, bCompress);

		AssetEntry entry{};
		entry.uuid = asset.GetUUID().GetRawData();
		std::memcpy(entry.type, asset.GetType(), sizeof(entry.type));
		entry.dataSize = assetData.size();
		entry.dataOffset = lastOffset;
		entry.bCompressed = bCompress;

		bundleData.resize(bundleData.size() + assetData.size());
		std::memcpy(bundleData.data() + lastOffset, assetData.data(), assetData.size());

		assetEntries.insert_or_assign(asset.GetUUID(), entry);
		return true;
	}
	SH_CORE_API auto AssetBundle::HasAsset(const UUID& uuid) const -> bool
	{
		return assetEntries.find(uuid) != assetEntries.end();
	}
	SH_CORE_API auto AssetBundle::GetAllAssetUUIDs() const -> std::vector<UUID>
	{
		std::vector<UUID> uuids;
		uuids.reserve(assetEntries.size());

		for (const auto& pair : assetEntries)
			uuids.push_back(pair.first);
		return uuids;
	}
	SH_CORE_API auto AssetBundle::SaveBundle(const std::filesystem::path& path) const -> bool
	{
		bundleStream.clear();

		if (bundleStream.is_open())
			bundleStream.close();

		bundleStream.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!bundleStream.is_open())
		{
			SH_ERROR_FORMAT(u8"Could not open file for writing: {}", path.u8string());
			return false;
		}

		Header header{};
		header.version = VERSION;
		header.numAssets = static_cast<uint32_t>(assetEntries.size());

		bundleStream.write(reinterpret_cast<const char*>(&header), sizeof(Header));

		for (const auto& pair : assetEntries)
		{
			const AssetEntry& entry = pair.second;
			bundleStream.write(reinterpret_cast<const char*>(&entry), sizeof(AssetEntry));
		}
		if (!bundleData.empty())
		{
			bundleStream.write(reinterpret_cast<const char*>(bundleData.data()), bundleData.size());
		}
		return true;
	}
	SH_CORE_API auto AssetBundle::LoadBundle(const std::filesystem::path& path) -> bool
	{
		bundleStream.clear();

		if (!bundleStream.is_open())
		{
			bundleStream.open(path, std::ios::binary | std::ios::in);
			if (!bundleStream.is_open())
			{
				SH_ERROR_FORMAT("Could not open file for reading: {}", path.u8string());
				return false;
			}
		}
		else
			bundleStream.seekg(0);

		assetEntries.clear();

		bundleStream.read(reinterpret_cast<char*>(&header), sizeof(Header));

		if (header.version != VERSION)
		{
			SH_WARN_FORMAT("AssetBundle version mismatch. Expected {}, found {}. Loading might fail.", VERSION, header.version);
		}

		for (uint32_t i = 0; i < header.numAssets; ++i)
		{
			AssetEntry entry{};
			bundleStream.read(reinterpret_cast<char*>(&entry), sizeof(AssetEntry));
			assetEntries.insert_or_assign(UUID{ entry.uuid }, entry);
		}

		bundlePath = path;
		return true;
	}
	SH_CORE_API auto AssetBundle::LoadAsset(const core::UUID& uuid) -> std::unique_ptr<Asset>
	{
		assert(!bundlePath.empty());

		bundleStream.clear();

		if (!bundleStream.is_open())
		{
			SH_ERROR("Call LoadBundle() first");
			return nullptr;
		}
		auto it = assetEntries.find(uuid);
		if (it == assetEntries.end())
		{
			SH_ERROR_FORMAT("Asset({}) is not exist in bundle", uuid.ToString());
			return nullptr;
		}
		const AssetEntry& entry = it->second;

		auto dataSectionOffset = sizeof(Header) + assetEntries.size() * sizeof(AssetEntry);
		bundleStream.seekg(dataSectionOffset + static_cast<std::streampos>(entry.dataOffset));

		std::vector<uint8_t> data(entry.dataSize);
		bundleStream.read(reinterpret_cast<char*>(data.data()), entry.dataSize);

		return AssetImporter::LoadFromMemory(data);
	}
	SH_CORE_API auto AssetBundle::GetVersion() const -> uint32_t
	{
		return header.version;
	}
}//namespace