#include "Game/PCH.h"
#include "EditorResource.h"
#include "EditorWorld.h"

#include "Game/TextureLoader.h"
#include "Game/ModelLoader.h"

namespace sh::editor
{
	SH_EDITOR_API void EditorResource::LoadAllAssets(EditorWorld& world)
	{
		game::TextureLoader texLoader{ world.renderer };

		folderIcon.Create(world.renderer, *world.textures.AddResource("FolderIcon", texLoader.Load("textures/folder.png")));
		fileIcon.Create(world.renderer, *world.textures.AddResource("FileIcon", texLoader.Load("textures/file.png")));
		meshIcon.Create(world.renderer, *world.textures.AddResource("MeshIcon", texLoader.Load("textures/meshIcon.png")));
		materialIcon.Create(world.renderer, *world.textures.AddResource("MaterialIcon", texLoader.Load("textures/MaterialIcon.png")));
	}

	SH_EDITOR_API auto EditorResource::GetIcon(Icon icon) const -> const game::GUITexture*
	{
		switch (icon)
		{
		case Icon::Folder:
			return &folderIcon;
		case Icon::File:
			return &fileIcon;
		case Icon::Mesh:
			return &meshIcon;
		case Icon::Material:
			return &materialIcon;
		default:
			return nullptr;
		}
	}

}