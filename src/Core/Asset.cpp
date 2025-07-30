#include "Asset.h"
namespace sh::core
{
	Asset::Asset(const char* type) :
		assetUUID(UUID::Generate()), type(type)
	{
	}
	Asset::Asset(const Asset& other) :
		assetUUID(other.assetUUID),
		type(other.type),
		data(other.data)
	{
	}
	Asset::Asset(Asset&& other) noexcept :
		assetUUID(std::move(other.assetUUID)),
		type(other.type),
		data(std::move(other.data))
	{
	}
	SH_CORE_API auto Asset::GetVersion() const -> uint32_t
	{
		return assetVersion;
	}
	SH_CORE_API auto Asset::GetAssetSize() const -> uint64_t
	{
		return sizeof(Header) + data.size();
	}
	SH_CORE_API auto Asset::GetAssetDataSize() const -> uint64_t
	{
		return data.size();
	}
	SH_CORE_API auto Asset::GetAssetUUID() const -> const UUID&
	{
		return assetUUID;
	}
	SH_CORE_API auto Asset::GetType() const -> const char*
	{
		return type;
	}
	SH_CORE_API void Asset::SetWriteTime(const std::filesystem::path& filePath)
	{
		if (std::filesystem::exists(filePath))
			writeTime = std::filesystem::last_write_time(filePath).time_since_epoch().count();
	}
	SH_CORE_API void Asset::SetWriteTime(int64_t time)
	{
		writeTime = time;
	}
	SH_CORE_API auto Asset::GetWriteTime() const -> int64_t
	{
		return writeTime;
	}
	SH_CORE_API auto Asset::IsEmpty() const -> bool
	{
		return data.empty();
	}
}//namespace