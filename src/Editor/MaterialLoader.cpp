#include "MaterialLoader.h"

#include "Core/FileSystem.h"
#include "Core/ISerializable.h"

#include "Render/ShaderPass.h"
#include "Render/Material.h"

namespace sh::editor
{
	SH_EDITOR_API MaterialLoader::MaterialLoader(const render::IRenderContext& context) :
		context(context)
	{
	}
	SH_EDITOR_API auto MaterialLoader::Load(std::string_view filename) -> render::Material*
	{
		auto file = core::FileSystem::LoadText(filename);
		if (!file.has_value())
			return nullptr;
		
		core::Json matJson{ core::Json::parse(file.value()) };

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