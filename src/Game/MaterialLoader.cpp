#include "MaterialLoader.h"
#include "MaterialAsset.h"

#include "Core/FileSystem.h"
#include "Core/ISerializable.h"
#include "Core/Logger.h"

#include "Render/ShaderPass.h"
#include "Render/Material.h"

namespace sh::game
{
	SH_GAME_API MaterialLoader::MaterialLoader(const render::IRenderContext& context) :
		context(context)
	{
	}
	SH_GAME_API auto MaterialLoader::Load(const std::filesystem::path& path) -> core::SObject*
	{
		auto file = core::FileSystem::LoadText(path);
		if (!file.has_value())
			return nullptr;
		
		const core::Json matJson = core::Json::parse(file.value());
		if (!matJson.contains("type"))
			return nullptr;
		if (matJson["type"].get<std::string>() != "Material")
			return nullptr;

		render::Material* mat = core::SObject::Create<render::Material>();
		mat->Deserialize(matJson);
		mat->Build(context);

		return mat;
	}
	SH_GAME_API auto MaterialLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), MaterialAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& matAsset = static_cast<const game::MaterialAsset&>(asset);

		const core::Json& matJson = matAsset.GetMaterialData();
		if (!matJson.contains("type"))
			return nullptr;
		if (matJson["type"].get<std::string>() != "Material")
			return nullptr;
		
		render::Material* mat = core::SObject::Create<render::Material>();
		mat->Deserialize(matJson);
		mat->Build(context);

		return mat;
	}
	SH_GAME_API auto MaterialLoader::GetAssetName() const -> const char*
	{
		return MaterialAsset::ASSET_NAME;
	}
}//namespace