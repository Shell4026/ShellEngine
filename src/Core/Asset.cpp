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
	SH_CORE_API void Asset::SetWriteTime(const std::filesystem::path& filePath)
	{
		if (std::filesystem::exists(filePath))
			writeTime = std::filesystem::last_write_time(filePath).time_since_epoch().count();
	}
	SH_CORE_API void Asset::SetWriteTime(int64_t time)
	{
		writeTime = time;
	}
}//namespace