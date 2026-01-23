#pragma once
#include "Export.h"
#include "ExplorerUI.h"

#include "Render/IRenderContext.h"

#include <filesystem>
#include <vector>
#include <string>
namespace sh::editor
{
	class FontGeneratorUI
	{
	public:
		SH_EDITOR_API FontGeneratorUI(const render::IRenderContext& ctx);

		SH_EDITOR_API void SetAssetPath(const std::filesystem::path& assetPath);

		SH_EDITOR_API void Clear();
		SH_EDITOR_API void Open();
		SH_EDITOR_API void Render();
	private:
		const render::IRenderContext& renderCtx;
		ExplorerUI explorer;

		std::filesystem::path assetPath;
		std::filesystem::path fontPath;
		std::vector<uint8_t> fontData;

		float fontSize = 32.f;
		int padding = 2;
		uint32_t atlasWidth = 1024;
		uint32_t atlasHeight = 1024;
		std::string str;

		bool bShow = false;
	};
}//namespace