#pragma once
#include "Export.h"
#include "Asset.h"

#include <memory>
namespace sh::core
{
	class AssetImporter
	{
	public:
		SH_CORE_API static auto Load(const std::filesystem::path& path) -> std::unique_ptr<Asset>;
		SH_CORE_API static auto LoadFromMemory(const std::vector<uint8_t>& data) -> std::unique_ptr<Asset>;
		SH_CORE_API static auto LoadFromMemory(const uint8_t* ptr, std::size_t size) -> std::unique_ptr<Asset>;
	};
}//namespace