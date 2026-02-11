#include "Asset/ScriptableObjectLoader.h"
#include "Asset/ScriptableObjectAsset.h"

#include "Core/FileSystem.h"
#include "Core/Reflection.hpp"

namespace sh::game
{
	SH_GAME_API ScriptableObjectLoader::ScriptableObjectLoader()
	{
	}
	SH_GAME_API auto ScriptableObjectLoader::Load(const std::filesystem::path& path) const -> core::SObject*
	{
		auto file = core::FileSystem::LoadText(path);
		if (!file.has_value())
			return nullptr;

		if (file.value().empty())
			return nullptr;
		const core::Json json = core::Json::parse(file.value());
		if (!json.contains("fullType"))
			return nullptr;

		const std::string& fullType = json["fullType"].get_ref<const std::string&>();

		ScriptableObject* const srpoPtr = ScriptableObject::Factory::GetInstance()->Create(fullType);
		if (srpoPtr != nullptr)
			srpoPtr->Deserialize(json);

		return srpoPtr;
	}
	SH_GAME_API auto ScriptableObjectLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ScriptableObjectAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& srpAsset = static_cast<const ScriptableObjectAsset&>(asset);

		const core::Json& json = srpAsset.GetSerializationData();
		if (!json.contains("fullType"))
			return nullptr;

		const std::string& fullType = json["fullType"].get_ref<const std::string&>();

		if (auto oldObj = core::SObjectManager::GetInstance()->GetSObject(asset.GetAssetUUID()); oldObj != nullptr)
		{
			ScriptableObject* oldSrpoPtr = core::reflection::Cast<ScriptableObject>(oldObj);
			if (oldSrpoPtr == nullptr)
				return nullptr;

			if (oldSrpoPtr->GetType().type.name != fullType)
				return nullptr;

			oldSrpoPtr->Deserialize(json);
			return oldSrpoPtr;
		}
		ScriptableObject* srpoPtr = ScriptableObject::Factory::GetInstance()->Create(fullType);
		if (srpoPtr != nullptr)
			srpoPtr->Deserialize(json);

		return srpoPtr;
	}
	SH_GAME_API auto ScriptableObjectLoader::GetAssetName() const -> const char*
	{
		return ScriptableObjectAsset::ASSET_NAME;
	}
}//namespace