#include "ShaderPassAsset.h"

#include <string>
namespace sh::game
{
	SH_GAME_API void ShaderPassAsset::SetAssetData() const
	{
		if (!passPtr.IsValid())
			return;

		const core::Json passJson = passPtr->Serialize();
		data = core::Json::to_cbor(passJson);
	}
	SH_GAME_API auto ShaderPassAsset::ParseAssetData() -> bool
	{
		const core::Json passJson = core::Json::from_cbor(data);

		return true;
	}
	ShaderPassAsset::ShaderPassAsset() :
		Asset("shad")
	{
	}
	ShaderPassAsset::ShaderPassAsset(const render::ShaderPass& pass) :
		Asset("shad"),
		passPtr(static_cast<const render::ShaderPass*>(&pass))
	{
		uuid = passPtr->GetUUID();
	}
	SH_GAME_API void ShaderPassAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::ShaderPass::GetStaticType())
			return;

		passPtr = static_cast<const render::ShaderPass*>(&obj);
		uuid = passPtr->GetUUID();
	}
}//namespace