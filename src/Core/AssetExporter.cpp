#include "AssetExporter.h"
#include "FileSystem.h"

#include "../External/lz4/lz4.h"

namespace sh::core
{
	SH_CORE_API auto AssetExporter::Save(const Asset& asset, const std::filesystem::path& path, bool bCompress) -> bool
	{
		std::vector<uint8_t> blob(SaveToMemory(asset, bCompress));

		return FileSystem::SaveBinary(blob, path);
	}
	SH_CORE_API auto AssetExporter::SaveToMemory(const Asset& asset, bool bCompress) -> std::vector<uint8_t>
	{
		asset.SetAssetData();

		Asset::Header header{};
		header.version = Asset::VERSION;
		std::memcpy(header.type, asset.GetType(), sizeof(header.type));
		header.uuid = asset.uuid.GetRawData();
		header.originalDataSize = asset.data.size();
		header.compressedDataSize = header.originalDataSize;
		header.writeTime = asset.writeTime;

		std::vector<uint8_t> compressedData;
		if (bCompress)
		{
			int compressedSize = 0;
			int maxCompressedSize = LZ4_compressBound(static_cast<int>(asset.data.size()));
			compressedData.resize(maxCompressedSize);

			compressedSize = LZ4_compress_default(
				reinterpret_cast<const char*>(asset.data.data()),
				reinterpret_cast<char*>(compressedData.data()),
				static_cast<int>(header.originalDataSize),
				maxCompressedSize
			);

			if (compressedSize <= 0) // 실패
			{
				header.compressedDataSize = header.originalDataSize;
				compressedData.assign(asset.data.begin(), asset.data.end());
			}
			else
				header.compressedDataSize = static_cast<uint64_t>(compressedSize);
		}
		const std::size_t blobSize = sizeof(Asset::Header) + header.compressedDataSize;

		std::vector<uint8_t> blob(blobSize, 0);
		std::memcpy(&blob[0], &header, sizeof(Asset::Header));
		if (!bCompress)
			std::memcpy(blob.data() + sizeof(Asset::Header), asset.data.data(), header.originalDataSize);
		else
			std::memcpy(blob.data() + sizeof(Asset::Header), compressedData.data(), header.compressedDataSize);

		return blob;
	}
}//namespace