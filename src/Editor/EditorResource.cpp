#include "EditorResource.h"
#include "EditorWorld.h"

#include "Game/TextureLoader.h"

namespace sh::editor
{
	SH_EDITOR_API void EditorResource::LoadAllAssets(EditorWorld& world)
	{
		this->world = &world;

		auto& ctx = *world.renderer.GetContext();
		game::TextureLoader texLoader{ ctx };

		folderIcon.Create(ctx, *world.textures.AddResource("FolderIcon", static_cast<render::Texture*>(texLoader.Load("textures/folder.png"))));
		fileIcon.Create(ctx, *world.textures.AddResource("FileIcon", static_cast<render::Texture*>(texLoader.Load("textures/file.png"))));
		meshIcon.Create(ctx, *world.textures.AddResource("MeshIcon", static_cast<render::Texture*>(texLoader.Load("textures/meshIcon.png"))));
		materialIcon.Create(ctx, *world.textures.AddResource("MaterialIcon", static_cast<render::Texture*>(texLoader.Load("textures/MaterialIcon.png"))));
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