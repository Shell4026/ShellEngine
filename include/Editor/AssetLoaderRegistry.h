#pragma once
#include "Editor/Export.h"
#include "AssetExtensions.h"

#include "Core/IAssetLoader.h"

#include <unordered_map>
#include <functional>
#include <filesystem>
namespace sh::editor
{
	class AssetLoaderRegistry
	{
	public:
		struct Importer
		{
			std::unique_ptr<core::IAssetLoader> loader;
			int priority;
			bool bObjDataInMeta;
		};
	public:
		SH_EDITOR_API void RegisterLoader(AssetExtensions::Type ext, std::unique_ptr<core::IAssetLoader>&& loader, int priority, bool bObjDataInMeta);
		SH_EDITOR_API auto GetLoader(AssetExtensions::Type ext) const -> const Importer*;
		SH_EDITOR_API void Clear();
	private:
		std::unordered_map<AssetExtensions::Type, Importer> loaders;
	};
}//namespace