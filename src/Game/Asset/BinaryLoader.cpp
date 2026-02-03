#include "Asset/BinaryLoader.h"
#include "Asset/BinaryAsset.h"

#include "Core/FileSystem.h"
#include "Core/Reflection.hpp"

namespace sh::game
{
	SH_GAME_API BinaryLoader::BinaryLoader()
	{
	}
	SH_GAME_API auto BinaryLoader::Load(const std::filesystem::path& path) -> core::SObject*
	{
		auto file = core::FileSystem::LoadBinary(path);
		if (!file.has_value())
			return nullptr;

		BinaryObject* binaryObj = core::SObject::Create<BinaryObject>();
		binaryObj->data = file.value();

		return binaryObj;
	}
	SH_GAME_API auto BinaryLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), BinaryAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& textAsset = static_cast<const BinaryAsset&>(asset);

		const std::vector<uint8_t>& rawData = textAsset.GetRawData();
		
		if (auto oldObj = core::SObjectManager::GetInstance()->GetSObject(asset.GetAssetUUID()); oldObj != nullptr)
		{
			BinaryObject* oldBinaryObj = core::reflection::Cast<BinaryObject>(oldObj);
			if (oldBinaryObj == nullptr)
				return nullptr;
			oldBinaryObj->data = rawData;
			return oldBinaryObj;
		}

		BinaryObject* binaryObj = core::SObject::Create<BinaryObject>();
		binaryObj->SetUUID(asset.GetAssetUUID());
		binaryObj->data = rawData;
		return binaryObj;
	}
	SH_GAME_API auto BinaryLoader::GetAssetName() const -> const char*
	{
		return BinaryAsset::ASSET_NAME;
	}
}//namespace