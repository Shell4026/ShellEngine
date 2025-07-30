#include "EditorResource.h"
#include "UI/Project.h"

#include "Game/TextureLoader.h"

namespace sh::editor
{
	SH_EDITOR_API void EditorResource::LoadAllAssets(Project& project)
	{
		auto& ctx = *project.renderer.GetContext();
		game::TextureLoader texLoader{ ctx };

		auto folderTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/folder.png"))));
		assert(folderTex != nullptr);
		auto fileTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/file.png"))));
		assert(fileTex != nullptr);
		auto meshTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/meshIcon.png"))));
		assert(meshTex != nullptr);
		auto materialTex = static_cast<render::Texture*>(project.loadedAssets.AddResource(core::UUID::Generate(), static_cast<render::Texture*>(texLoader.Load("textures/MaterialIcon.png"))));
		assert(materialTex != nullptr);

		folderIcon.Create(ctx, *folderTex);
		fileIcon.Create(ctx, *fileTex);
		meshIcon.Create(ctx, *meshTex);
		materialIcon.Create(ctx, *materialTex);
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