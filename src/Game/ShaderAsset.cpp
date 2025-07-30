#include "ShaderAsset.h"

namespace sh::game
{
	SH_GAME_API void ShaderAsset::SetAssetData() const
	{
		if (!shaderPtr.IsValid())
			return;

		const core::Json shaderObjJson = shaderPtr->Serialize();
		data = core::Json::to_cbor(shaderObjJson);
	}
	SH_GAME_API auto ShaderAsset::ParseAssetData() -> bool
	{
		shaderObjJson = core::Json::from_cbor(data);
		return true;
	}
	ShaderAsset::ShaderAsset() :
		Asset(ASSET_NAME)
	{
	}
	ShaderAsset::ShaderAsset(const render::Shader& shader) :
		Asset(ASSET_NAME),
		shaderPtr(static_cast<const render::Shader*>(&shader))
	{
		assetUUID = shaderPtr->GetUUID();
	}
	SH_GAME_API void ShaderAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Shader::GetStaticType())
			return;

		shaderPtr = static_cast<const render::Shader*>(&obj);
		assetUUID = shaderPtr->GetUUID();
	}
	SH_GAME_API auto ShaderAsset::GetShaderObjectJson() const -> const core::Json&
	{
		return shaderObjJson;
	}
}//namespace