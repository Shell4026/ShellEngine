#include "AssetImporter.h"
#include "Factory.hpp"
#include "FileSystem.h"

#include "../External/lz4/lz4.h"

#include <cstring>
namespace sh::core
{
	SH_CORE_API auto AssetImporter::Load(const std::filesystem::path& path) -> std::unique_ptr<Asset>
	{
		auto blobOpt = FileSystem::LoadBinary(path);
		if (!blobOpt.has_value())
			return nullptr;
		if (blobOpt.value().size() < sizeof(Asset::Header))
			return nullptr;
		std::vector<uint8_t> blob = std::move(*blobOpt);

		return LoadFromMemory(blob);
	}
	SH_CORE_API auto AssetImporter::LoadFromMemory(const std::vector<uint8_t>& data) -> std::unique_ptr<Asset>
	{
		if (data.empty()) 
			return nullptr;
		return LoadFromMemory(data.data(), data.size());
	}
	SH_CORE_API auto AssetImporter::LoadFromMemory(const uint8_t* ptr, std::size_t size) -> std::unique_ptr<Asset>
	{
		Asset::Header header{};
		std::memcpy(&header, ptr, sizeof(Asset::Header));

		if (size != sizeof(Asset::Header) + header.compressedDataSize)
			return nullptr;

		char typeStr[5];
		std::memcpy(typeStr, header.type, sizeof(header.type));
		typeStr[4] = '\0';

		std::unique_ptr<Asset> asset{ Factory<Asset>::GetInstance()->Create(typeStr) };
		if (asset == nullptr)
			return nullptr;

		asset->assetVersion = header.version;
		asset->uuid = header.uuid;
		asset->data.resize(header.originalDataSize, 0);
		asset->writeTime = header.writeTime;

		const uint8_t* dataPtr = ptr + sizeof(Asset::Header);

		bool bCompressed = header.compressedDataSize != header.originalDataSize;
		if (!bCompressed)
			std::memcpy(asset->data.data(), dataPtr, asset->data.size());
		else
		{
			int decompressedSize = LZ4_decompress_safe(
				reinterpret_cast<const char*>(dataPtr),
				reinterpret_cast<char*>(asset->data.data()),
				static_cast<int>(header.compressedDataSize),
				static_cast<int>(header.originalDataSize)
			);

			if (decompressedSize < 0 || static_cast<uint64_t>(decompressedSize) != header.originalDataSize)
			{
				asset->data.clear();
				return nullptr;
			}
		}
		if (!asset->ParseAssetData())
			return nullptr;
		return asset;
	}
}//namespace