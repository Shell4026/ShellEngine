#include "Asset/ComputeShaderAsset.h"

namespace sh::game
{
	ComputeShaderAsset::ComputeShaderAsset() :
		Asset(ASSET_NAME)
	{
	}
	ComputeShaderAsset::ComputeShaderAsset(const render::ComputeShader& shader) :
		Asset(ASSET_NAME),
		shaderPtr(static_cast<const render::ComputeShader*>(&shader))
	{
		assetUUID = shaderPtr->GetUUID();
	}
	SH_GAME_API void ComputeShaderAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::ComputeShader::GetStaticType())
			return;

		shaderPtr = static_cast<const render::ComputeShader*>(&obj);
		assetUUID = shaderPtr->GetUUID();
	}
	SH_GAME_API void ComputeShaderAsset::SetAssetData() const
	{
		if (!shaderPtr.IsValid())
			return;

		const core::Json shaderObjJson = shaderPtr->Serialize();
		data = core::Json::to_cbor(shaderObjJson);
	}
	SH_GAME_API auto ComputeShaderAsset::ParseAssetData() -> bool
	{
		shaderObjJson = core::Json::from_cbor(data);
		return true;
	}
}//namespace
