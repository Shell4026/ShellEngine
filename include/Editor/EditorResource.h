#pragma once

#include "Export.h"

#include "Core/Singleton.hpp"

#include "Game/GUITexture.h"

namespace sh::editor
{
	class EditorWorld;

	class EditorResource : public core::Singleton<EditorResource>
	{
	private:
		EditorWorld* world;

		game::GUITexture folderIcon, fileIcon, meshIcon, materialIcon;
	public:
		enum class Icon
		{
			Folder,
			File,
			Mesh,
			Material
		};
	public:
		SH_EDITOR_API void LoadAllAssets(EditorWorld& world);

		SH_EDITOR_API auto GetIcon(Icon icon) const -> const game::GUITexture*;
	};
}//namespace