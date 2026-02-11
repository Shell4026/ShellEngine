#include "AssetLoaderRegistry.h"

#include "Core/Logger.h"
namespace sh::editor
{
	SH_EDITOR_API void AssetLoaderRegistry::RegisterLoader(AssetExtensions::Type ext, std::unique_ptr<core::IAssetLoader>&& loader, int priority, bool bObjDataInMeta)
	{
		auto it = loaders.find(ext);
		if (it != loaders.end())
		{
			SH_ERROR_FORMAT("Asset loader({}) is already exist!", AssetExtensions::ToString(ext));
			return;
		}
		Importer importer{};
		importer.loader = std::move(loader);
		importer.priority = priority;
		importer.bObjDataInMeta = bObjDataInMeta;
		loaders.insert({ ext, std::move(importer) });
	}
	SH_EDITOR_API auto AssetLoaderRegistry::GetLoader(AssetExtensions::Type ext) const -> const Importer*
	{
		if (ext == AssetExtensions::Type::None)
			return nullptr;

		auto it = loaders.find(ext);
		if (it == loaders.end())
		{
			SH_ERROR_FORMAT("Asset loader is not exist! (type: {})", AssetExtensions::ToString(ext));
			return nullptr;
		}
		return &it->second;
	}
	SH_EDITOR_API void AssetLoaderRegistry::Clear()
	{
		loaders.clear();
	}
}//namespace