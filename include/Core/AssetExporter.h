#pragma once
#include "Export.h"
#include "Asset.h"

namespace sh::core
{
	class AssetExporter
	{
	public:
		SH_CORE_API static auto Save(const Asset& asset, const std::filesystem::path& path, bool bCompress) -> bool;
		SH_CORE_API static auto SaveToMemory(const Asset& asset, bool bCompress) -> std::vector<uint8_t>;
	};
}//namespace