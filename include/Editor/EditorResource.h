#pragma once
#include "Export.h"
#include "Core/Singleton.hpp"
#include "Core/SContainer.hpp"

#include "Game/GUITexture.h"

namespace sh::render
{
	class Shader;
	class Material;
	class Texture;
}
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
	public:
		enum class Icon
		{
			Folder,
			File,
			Mesh,
			Material
		};
	public:
		SH_EDITOR_API void LoadAllAssets(Project& world);

		SH_EDITOR_API auto GetIcon(Icon icon) const -> const game::GUITexture*;
		SH_EDITOR_API auto GetShader(const std::string& name) -> render::Shader*;
		SH_EDITOR_API auto GetMaterial(const std::string& name) -> render::Material*;
		SH_EDITOR_API auto GetTexture(const std::string& name) -> render::Texture*;
	};
}//namespace