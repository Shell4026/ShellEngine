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
	public:
		enum class Icon
		{
			Folder,
			File,
			Mesh,
			Material,
			Image,
			Shader,
			World,
			Prefab
		};
	public:
		SH_EDITOR_API void LoadAllAssets(Project& project);

		SH_EDITOR_API auto GetIcon(Icon icon) const -> const game::GUITexture*;
		SH_EDITOR_API auto GetShader(const std::string& name) -> render::Shader*;
		SH_EDITOR_API auto GetMaterial(const std::string& name) -> render::Material*;
		SH_EDITOR_API auto GetTexture(const std::string& name) -> render::Texture*;
		SH_EDITOR_API auto GetModel(const std::string& name) -> render::Model*;

		SH_EDITOR_API void ExtractAllAssetToLibrary(Project& project);
	private:
		game::GUITexture folderIcon, fileIcon, meshIcon, materialIcon, imageIcon, shaderIcon, worldIcon, prefabIcon;

		core::SMap<std::string, render::Shader*> shaders;
		core::SMap<std::string, render::Material*> materials;
		core::SMap<std::string, render::Texture*> textures;
		core::SMap<std::string, render::Model*> models;
	};
}//namespace