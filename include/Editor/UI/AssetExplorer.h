#pragma once
#include "Editor/Export.h"

namespace sh::editor
{
	class EditorWorld;
	class AssetExplorer
	{
	private:
		const EditorWorld& world;
	public:
		enum class AssetType
		{
			Mesh,
			Texture
		} type;
	public:
		SH_EDITOR_API AssetExplorer(const EditorWorld& world);

		SH_EDITOR_API void SetAssetType(AssetType type);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();
	};
}//namespace