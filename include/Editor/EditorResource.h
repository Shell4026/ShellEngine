#pragma once
#include "Export.h"
#include "Core/Singleton.hpp"
#include "Core/SContainer.hpp"

#include "Game/GUITexture.h"

#include "Render/Shader.h"
#include "Render/Material.h"
#include "Render/Texture.h"
#include "Render/Model.h"
namespace sh::editor
{
	class Project;

	class EditorResource : public core::Singleton<EditorResource>
	{
	private:
		game::GUITexture folderIcon, fileIcon, meshIcon, materialIcon;

		core::SMap<std::string, render::Shader*> shaders;
		core::SMap<std::string, render::Material*> materials;
		core::SMap<std::string, render::Texture*> textures;
		core::SMap<std::string, render::Model*> models;
	public:
		enum class Icon
		{
			Folder,
			File,
			Mesh,
			Material
		};
	public:
		SH_EDITOR_API void LoadAllAssets(Project& project);

		SH_EDITOR_API auto GetIcon(Icon icon) const -> const game::GUITexture*;
		SH_EDITOR_API auto GetShader(const std::string& name) -> render::Shader*;
		SH_EDITOR_API auto GetMaterial(const std::string& name) -> render::Material*;
		SH_EDITOR_API auto GetTexture(const std::string& name) -> render::Texture*;
		SH_EDITOR_API auto GetModel(const std::string& name) -> render::Model*;

		SH_EDITOR_API void ExtractAllAssetToLibrary(Project& project);
	};
}//namespace