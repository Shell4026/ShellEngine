#pragma once

#include <filesystem>
namespace sh::core
{
	class SObject;
	class Asset;
	class IAssetLoader
	{
	public:
		virtual ~IAssetLoader() = default;

		virtual auto Load(const std::filesystem::path& filePath) const -> SObject* = 0;
		virtual auto Load(const core::Asset& asset) const -> SObject* = 0;
		virtual auto GetAssetName() const -> const char* = 0;
	};
}