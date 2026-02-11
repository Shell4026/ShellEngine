#include "Asset/ScriptableObjectAsset.h"

#include "Core/Reflection.hpp"
#include "Core/Factory.hpp"
namespace sh::game
{
	ScriptableObjectAsset::ScriptableObjectAsset() :
		Asset(ASSET_NAME)
	{

	}
	ScriptableObjectAsset::ScriptableObjectAsset(const ScriptableObject& binaryObj) :
		Asset(ASSET_NAME)
	{
		objPtr = &binaryObj;
		assetUUID = objPtr->GetUUID();
	}
	SH_GAME_API void ScriptableObjectAsset::SetAsset(const core::SObject& obj)
	{
		const ScriptableObject* objPtr = core::reflection::Cast<const ScriptableObject>(&obj);
		if (objPtr == nullptr)
			return;

		this->objPtr = objPtr;
		assetUUID = objPtr->GetUUID();
	}
	SH_GAME_API void ScriptableObjectAsset::SetAssetData() const
	{
		if (!objPtr.IsValid())
			return;

		data = core::Json::to_bson(objPtr->Serialize());
	}
	SH_GAME_API auto ScriptableObjectAsset::ParseAssetData() -> bool
	{
		if (data.empty())
			return false;

		objPtr.Reset();
		serializationData = core::Json::from_bson(data);

		return true;
	}
}//namespace