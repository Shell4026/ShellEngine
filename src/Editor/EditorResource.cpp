#include "EditorResource.h"
#include "EditorWorld.h"
#include "TextureLoader.h"
#include "MeshLoader.h"

namespace sh::editor
{
	SH_EDITOR_API void EditorResource::LoadAllAssets(EditorWorld& world)
	{
		auto& ctx = *world.renderer.GetContext();
		TextureLoader texLoader{ ctx };

		folderIcon.Create(ctx, *world.textures.AddResource("FolderIcon", texLoader.Load("textures/folder.png")));
		fileIcon.Create(ctx, *world.textures.AddResource("FileIcon", texLoader.Load("textures/file.png")));
		meshIcon.Create(ctx, *world.textures.AddResource("MeshIcon", texLoader.Load("textures/meshIcon.png")));
		materialIcon.Create(ctx, *world.textures.AddResource("MaterialIcon", texLoader.Load("textures/MaterialIcon.png")));
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