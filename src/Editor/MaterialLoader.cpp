#include "MaterialLoader.h"

#include "Core/FileSystem.h"
#include "Core/ISerializable.h"
#include "Core/Logger.h"

#include "Render/ShaderPass.h"
#include "Render/Material.h"

namespace sh::editor
{
	SH_EDITOR_API MaterialLoader::MaterialLoader(const render::IRenderContext& context) :
		context(context)
	{
	}
	SH_EDITOR_API auto MaterialLoader::Load(const std::filesystem::path& path) -> render::Material*
	{
		auto file = core::FileSystem::LoadText(path);
		if (!file.has_value())
			return nullptr;
		
		core::Json matJson = core::Json::parse(file.value());
		if (!matJson.contains("type"))
			return nullptr;
		if (matJson["type"].get<std::string>() != "Material")
			return nullptr;

		render::Material* mat = core::SObject::Create<render::Material>();
		mat->Deserialize(matJson);
		mat->Build(context);

		return mat;
	}
}//namespace